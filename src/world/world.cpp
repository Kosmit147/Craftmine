#include "world.hpp"

#include <thread>

#include "assets.hpp"

using namespace std::chrono_literals;

const usize World::max_generate_chunk_tasks = std::max(std::thread::hardware_concurrency(), 4u);
const usize World::max_update_chunk_mesh_tasks = max_generate_chunk_tasks;

std::unordered_map<ChunkPosition, Chunk> World::_map;
std::unordered_map<ChunkPosition, zth::EntityHandle> World::_entity_map;

std::queue<ChunkPosition> World::_generate_chunk_requests;
std::queue<std::future<GenerateChunkResults>> World::_generate_chunk_tasks;

std::queue<ChunkPosition> World::_update_chunk_mesh_requests;
std::queue<std::future<UpdateChunkMeshResults>> World::_update_chunk_mesh_tasks;

std::shared_ptr<zth::gl::Texture2D> World::_blocks_texture;
std::shared_ptr<zth::Material> World::_chunk_material;

zth::Timer World::_generate_chunks_timer;

auto World::init(i32 distance) -> void
{
    _blocks_texture = std::make_shared<zth::gl::Texture2D>(zth::gl::Texture2D::from_file_data(
        blocks_texture_data, zth::gl::TextureParams{ .mag_filter = zth::gl::TextureMagFilter::nearest }));
    _chunk_material = std::make_shared<zth::Material>(zth::Material{ .diffuse_map = _blocks_texture });

    // Generate chunks in a spiral pattern starting from the center.

    // The loop below doesn't handle { 0, 0 } coordinates.
    if (distance >= 0)
        request_to_generate_new_chunk({ 0, 0 });

    for (i32 i = 1; i <= distance; i++)
    {
        // Top Row.
        for (i32 x = -i; x < i; x++)
            request_to_generate_new_chunk({ x, i });

        // Right Column.
        for (i32 z = i; z > -i; z--)
            request_to_generate_new_chunk({ i, z });

        // Bottom Row.
        for (i32 x = i; x > -i; x--)
            request_to_generate_new_chunk({ x, -i });

        // Left Column.
        for (i32 z = -i; z < i; z++)
            request_to_generate_new_chunk({ -i, z });
    }

    auto side = distance + 1 + distance;
    ZTH_INFO("Number of chunks: {}", side * side);
    _generate_chunks_timer.reset();
}

auto World::update(zth::Scene& scene) -> void
{
    // Process generate chunk requests.
    while (!_generate_chunk_requests.empty() && _generate_chunk_tasks.size() < max_generate_chunk_tasks)
    {
        auto chunk_position = _generate_chunk_requests.front();
        _generate_chunk_tasks.push(
            std::async(std::launch::async, [chunk_position] { return generate_new_chunk(chunk_position); }));
        _generate_chunk_requests.pop();
    }

    // Get results from generate chunk tasks.
    if (!_generate_chunk_tasks.empty())
    {
        auto& pending_task = _generate_chunk_tasks.front();

        if (pending_task.wait_for(0s) == std::future_status::ready)
        {
            auto [chunk_position, chunk_data] = pending_task.get();

            auto [kv, success] = _map.try_emplace(chunk_position, std::move(chunk_data));
            ZTH_ASSERT(success);
            auto& [_, chunk] = *kv;
            create_chunk_entity(scene, chunk, chunk_position);

            request_to_update_chunk_mesh(chunk_position);
            request_to_update_neighboring_chunks_meshes(chunk_position);
            _generate_chunk_tasks.pop();
        }
    }

    // Process update chunk mesh requests.
    while (!_update_chunk_mesh_requests.empty() && _update_chunk_mesh_tasks.size() < max_update_chunk_mesh_tasks)
    {
        auto chunk_position = _update_chunk_mesh_requests.front();

        if (auto chunk = get_chunk(chunk_position))
        {
            UpdateChunkMeshInputs inputs = {
                .chunk_data = chunk->get().data(),
                .neighbors = get_neighbors(chunk_position),
            };

            _update_chunk_mesh_tasks.push(std::async(std::launch::async, [chunk_position, input = std::move(inputs)] {
                return update_chunk_mesh(chunk_position, input);
            }));
        }

        _update_chunk_mesh_requests.pop();
    }

    // Get results from update chunk mesh tasks.
    if (!_update_chunk_mesh_tasks.empty())
    {
        auto& pending_task = _update_chunk_mesh_tasks.front();

        if (pending_task.wait_for(0s) == std::future_status::ready)
        {
            auto [chunk_position, chunk_mesh] = pending_task.get();

            if (auto chunk = get_chunk(chunk_position))
            {
                chunk->get().upload_mesh(chunk_mesh);
                update_chunk_entity_mesh(*chunk, chunk_position);
            }

            _update_chunk_mesh_tasks.pop();
        }
    }

    if (!_all_chunks_generated && _generate_chunk_requests.empty() && _generate_chunk_tasks.empty())
    {
        auto ms_to_s = [](double value) -> double { return value / 1000.0; };
        auto time = _generate_chunks_timer.elapsed_ms();
        ZTH_INFO("Total time to generate chunks: {} ms ({:.2f} s).", time, ms_to_s(time));
        _all_chunks_generated = true;
    }
}

auto World::shut_down() -> void
{
    _map.clear();

    _generate_chunk_requests = {}; // Clear the queue.
    _generate_chunk_tasks = {};    // Clear the queue.

    _update_chunk_mesh_requests = {}; // Clear the queue.
    _update_chunk_mesh_tasks = {};    // Clear the queue.

    _blocks_texture.reset();
    _chunk_material.reset();
}

auto World::get_chunk(ChunkPosition position) -> Optional<Reference<Chunk>>
{
    if (auto kv = _map.find(position); kv != _map.end())
    {
        auto& [_, value] = *kv;
        return value;
    }

    return nil;
}

auto World::get_chunk_unchecked(ChunkPosition position) noexcept -> Chunk&
{
    ZTH_ASSERT(_map.contains(position));
    return _map.at(position);
}

auto World::get_neighbors(ChunkPosition position) -> NeighboringChunksData
{
    auto backward_chunk = get_chunk({ position.x, position.z + 1 });
    auto forward_chunk = get_chunk({ position.x, position.z - 1 });
    auto left_chunk = get_chunk({ position.x - 1, position.z });
    auto right_chunk = get_chunk({ position.x + 1, position.z });

    return NeighboringChunksData{
        .backward_chunk_data = backward_chunk ? backward_chunk->get().data() : nullptr,
        .forward_chunk_data = forward_chunk ? forward_chunk->get().data() : nullptr,
        .left_chunk_data = left_chunk ? left_chunk->get().data() : nullptr,
        .right_chunk_data = right_chunk ? right_chunk->get().data() : nullptr,
    };
}

auto World::get_chunk_entity(ChunkPosition position) -> Optional<zth::EntityHandle>
{
    if (auto kv = _entity_map.find(position); kv != _entity_map.end())
    {
        auto& [_, value] = *kv;
        return value;
    }

    return nil;
}

auto World::get_chunk_entity_unchecked(ChunkPosition position) noexcept -> zth::EntityHandle&
{
    ZTH_ASSERT(_entity_map.contains(position));
    return _entity_map.at(position);
}

auto World::request_to_generate_new_chunk(ChunkPosition position) -> void
{
    _generate_chunk_requests.push(position);
}

auto World::request_to_update_chunk_mesh(ChunkPosition position) -> void
{
    _update_chunk_mesh_requests.push(position);
}

auto World::request_to_update_neighboring_chunks_meshes(ChunkPosition position) -> void
{
    _update_chunk_mesh_requests.push({ position.x + 1, position.z });
    _update_chunk_mesh_requests.push({ position.x - 1, position.z });
    _update_chunk_mesh_requests.push({ position.x, position.z + 1 });
    _update_chunk_mesh_requests.push({ position.x, position.z - 1 });
}

auto World::generate_new_chunk(ChunkPosition position) -> GenerateChunkResults
{
    // @multithreaded

    return GenerateChunkResults{ .position = position, .chunk_data = ChunkGenerator::generate(position) };
}

auto World::update_chunk_mesh(ChunkPosition position, const UpdateChunkMeshInputs& inputs) -> UpdateChunkMeshResults
{
    // @multithreaded

    return UpdateChunkMeshResults{
        .position = position,
        .mesh = inputs.chunk_data->generate_mesh(inputs.neighbors),
    };
}

auto World::create_chunk_entity(zth::Scene& scene, const Chunk& chunk, ChunkPosition position) -> void
{
    auto entity = scene.create_entity("Chunk");

    auto world_position = glm::vec3{ Chunk::to_world_x(position.x), 0.0f, Chunk::to_world_z(position.z) };
    entity.transform().set_translation(world_position);

    entity.emplace_or_replace<zth::QuadMeshComponent>(chunk.mesh());
    entity.emplace_or_replace<zth::MaterialComponent>(_chunk_material);

    auto [_, success] = _entity_map.try_emplace(position, entity);
    ZTH_ASSERT(success);
}

auto World::update_chunk_entity_mesh(const Chunk& chunk, ChunkPosition position) -> void
{
    auto entity_handle = get_chunk_entity_unchecked(position);
    auto& [mesh] = entity_handle.get<zth::QuadMeshComponent>();
    mesh = chunk.mesh();
}
