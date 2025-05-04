#include "world.hpp"

#include <thread>

#include "assets.hpp"

using namespace std::chrono_literals;

const usize World::max_generate_chunks_tasks = std::thread::hardware_concurrency();
const usize World::max_chunks_to_create_each_frame = max_generate_chunks_tasks;

std::unordered_map<ChunkPosition, Chunk> World::_map;

std::queue<ChunkPosition> World::_generate_chunk_requests;
std::queue<std::future<NewChunkParams>> World::_generate_chunk_tasks;

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
        _generate_chunk_requests.push({ 0, 0 });

    for (i32 i = 1; i <= distance; i++)
    {
        // Top Row.
        for (i32 x = -i; x < i; x++)
            _generate_chunk_requests.push({ x, i });

        // Right Column.
        for (i32 z = i; z > -i; z--)
            _generate_chunk_requests.push({ i, z });

        // Bottom Row.
        for (i32 x = i; x > -i; x--)
            _generate_chunk_requests.push({ x, -i });

        // Left Column.
        for (i32 z = -i; z < i; z++)
            _generate_chunk_requests.push({ -i, z });
    }

    auto side = distance + 1 + distance;
    ZTH_INFO("Number of chunks: {}", side * side);
    _generate_chunks_timer.reset();
}

auto World::update(zth::Scene& scene) -> void
{
    while (!_generate_chunk_requests.empty() && _generate_chunk_tasks.size() < max_generate_chunks_tasks)
    {
        auto chunk_position = _generate_chunk_requests.front();
        _generate_chunk_tasks.push(std::async(std::launch::async, [=] { return generate_new_chunk(chunk_position); }));
        _generate_chunk_requests.pop();
    }

    for (usize i = 0; i < max_chunks_to_create_each_frame && !_generate_chunk_tasks.empty(); i++)
    {
        auto& pending_task = _generate_chunk_tasks.front();

        if (pending_task.wait_for(0s) == std::future_status::ready)
        {
            auto [chunk_position, chunk_data, chunk_mesh] = pending_task.get();
            auto [kv, success] = _map.try_emplace(chunk_position, std::move(chunk_data));
            ZTH_ASSERT(success);
            auto& [_, chunk] = *kv;
            chunk.upload_mesh(chunk_mesh);
            create_chunk_entity(scene, chunk, chunk_position);

            // @todo: Update neighboring chunks' meshes.

            _generate_chunk_tasks.pop();
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

    _blocks_texture.reset();
    _chunk_material.reset();
}

auto World::generate_new_chunk(ChunkPosition position) -> NewChunkParams
{
    // @multithreaded

    auto chunk_data = ChunkGenerator::generate(position);
    auto mesh = chunk_data.generate_mesh();

    return NewChunkParams{
        .position = position,
        .chunk_data = std::move(chunk_data),
        .mesh = std::move(mesh),
    };
}

auto World::create_chunk_entity(zth::Scene& scene, const Chunk& chunk, ChunkPosition position) -> void
{
    auto entity = scene.create_entity("Chunk");

    auto world_position = glm::vec3{ Chunk::to_world_x(position.x), 0.0f, Chunk::to_world_z(position.z) };
    entity.transform().set_translation(world_position);

    entity.emplace_or_replace<zth::QuadMeshComponent>(chunk.mesh());
    entity.emplace_or_replace<zth::MaterialComponent>(_chunk_material);
}
