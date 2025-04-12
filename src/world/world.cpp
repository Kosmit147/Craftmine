#include "world.hpp"

#include <future>

std::unordered_map<ChunkPosition, Chunk> World::_world;
std::unique_ptr<zth::Material> World::_chunk_material;
zth::Scene* World::_scene = nullptr;

auto World::init(zth::Scene& scene) -> void
{
    _scene = &scene;
    _chunk_material = std::make_unique<zth::Material>();

    generate(50, 50);
}

auto World::shut_down() -> void
{
    _world.clear();
}

auto World::generate(i32 x_chunks, i32 z_chunks) -> void
{
    auto offset_x = x_chunks / 2;
    auto offset_z = z_chunks / 2;

    const auto xs = std::views::iota(0 - offset_x, x_chunks - offset_x);
    const auto zs = std::views::iota(0 - offset_z, z_chunks - offset_z);
    const auto coords = std::views::cartesian_product(xs, zs);
    const auto chunk_count = coords.size();

    ZTH_INFO("Number of chunks: {}", chunk_count);
    zth::Timer total_timer;

    double generate_chunks_time;

#define MULTI_THREADED 1

    {
        zth::Timer timer;

#if MULTI_THREADED
        zth::TemporaryVector<std::future<std::pair<ChunkPosition, std::unique_ptr<ChunkData>>>> futures;
        futures.reserve(chunk_count);

        for (auto [x, z] : coords)
        {
            futures.push_back(std::async(std::launch::async, [=] {
                return std::make_pair(ChunkPosition{ x, z }, ChunkGenerator::generate({ x, z }));
            }));
        }

        for (auto& future : futures)
        {
            auto pair = future.get();
            auto& position = pair.first;
            auto& chunk = pair.second;
            _world.try_emplace(position, std::move(chunk));
        }
#else
        for (auto [x, z] : coords)
            _chunks.emplace_back(ChunkGenerator::generate(x, z));
#endif
        generate_chunks_time = timer.elapsed_ms();
    }

    double generate_chunk_meshes_time;

    zth::TemporaryVector<std::pair<ChunkPosition, ChunkMesh>> chunk_meshes;
    chunk_meshes.reserve(chunk_count);

    {
        zth::Timer timer;
#if MULTI_THREADED
        zth::TemporaryVector<std::future<std::pair<ChunkPosition, ChunkMesh>>> futures;
        futures.reserve(chunk_count);

        for (auto& [key, value] : _world)
        {
            futures.push_back(
                std::async(std::launch::async, [&] { return std::make_pair(key, value.generate_mesh()); }));
        }

        for (auto& future : futures)
        {
            auto pair = future.get();
            auto& position = pair.first;
            auto& chunk_mesh = pair.second;
            chunk_meshes.emplace_back(position, std::move(chunk_mesh));
        }
#else
        for (auto& chunk : _chunks)
            meshes.push_back(chunk.generate_mesh());
#endif
        generate_chunk_meshes_time = timer.elapsed_ms();
    }

    double upload_chunk_meshes_time;

    {
        zth::Timer timer;

        for (auto& pair : chunk_meshes)
        {
            auto& position = pair.first;
            auto& mesh = pair.second;
            auto& chunk = _world.at(position);
            chunk.upload_mesh(mesh);
        }

        upload_chunk_meshes_time = timer.elapsed_ms();
    }

    double create_chunk_entities_time;

    {
        zth::Timer timer;

        for (auto& kv : _world)
        {
            auto& position = kv.first;
            auto& chunk = kv.second;
            create_chunk_entity(chunk, position);
        }

        create_chunk_entities_time = timer.elapsed_ms();
    }

    auto total_time = total_timer.elapsed_ms();

    auto seconds = [](double value) { return value / 1000.0; };

    ZTH_INFO("Time to generate chunks: {} ms ({:.2f} s).", generate_chunks_time, seconds(generate_chunks_time));
    ZTH_INFO("Time to generate chunk meshes: {} ms ({:.2f} s).", generate_chunk_meshes_time,
             seconds(generate_chunk_meshes_time));
    ZTH_INFO("Time to upload chunk meshes: {} ms ({:.2f} s).", upload_chunk_meshes_time,
             seconds(upload_chunk_meshes_time));
    ZTH_INFO("Time to create chunk entities: {} ms ({:.2f} s).", create_chunk_entities_time,
             seconds(create_chunk_entities_time));
    ZTH_INFO("Total time: {} ms ({:.2f} s).", total_time, seconds(total_time));
}

auto World::create_chunk_entity(const Chunk& chunk, ChunkPosition position) -> void
{
    auto entity = _scene->create_entity("Chunk");

    auto world_position = glm::vec3{ Chunk::to_world_x(position.x), 0.0f, Chunk::to_world_z(position.z) };
    entity.transform().set_translation(world_position);

    entity.emplace_or_replace<zth::MeshComponent>(&chunk.mesh());
    entity.emplace_or_replace<zth::MaterialComponent>(&*_chunk_material);
}
