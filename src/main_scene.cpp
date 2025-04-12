#include "main_scene.hpp"

#include <future>

#include "scripts/player.hpp"

auto MainScene::on_load() -> void
{
    _camera.emplace<zth::CameraComponent>(zth::CameraComponent{
        .aspect_ratio = zth::Window::aspect_ratio(),
        .fov = glm::radians(45.0f),
        .near = 0.1f,
        .far = 1000.0f,
    });

    _camera.emplace<zth::ScriptComponent>(std::make_unique<scripts::Player>());

    _directional_light.emplace<zth::LightComponent>(zth::DirectionalLight{});
    _directional_light.transform().set_direction(glm::normalize(glm::vec3{ -0.35f, -1.0f, -0.35f }));

    _camera.transform().set_translation(glm::vec3{ 0.0f, 120.0f, 0.0f });
    _camera.transform().set_direction(zth::math::world_backward);

    generate_world(50, 50);
}

auto MainScene::generate_world(i32 x_chunks, i32 z_chunks) -> void
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
        zth::TemporaryVector<std::future<std::unique_ptr<ChunkData>>> futures;
        futures.reserve(chunk_count);

        for (auto [x, z] : coords)
            futures.push_back(std::async(std::launch::async, [=] { return ChunkGenerator::generate(x, z); }));

        for (auto& future : futures)
            _chunks.emplace_back(future.get());
#else
        for (auto [x, z] : coords)
            _chunks.emplace_back(ChunkGenerator::generate(x, z));
#endif
        generate_chunks_time = timer.elapsed_ms();
    }

    double generate_chunk_meshes_time;

    zth::TemporaryVector<ChunkMesh> chunk_meshes;
    chunk_meshes.reserve(chunk_count);

    {
        zth::Timer timer;
#if MULTI_THREADED
        zth::TemporaryVector<std::future<ChunkMesh>> futures;
        futures.reserve(chunk_count);

        for (auto& chunk : _chunks)
            futures.push_back(std::async(std::launch::async, [&] { return chunk.generate_mesh(); }));

        for (auto& future : futures)
            chunk_meshes.push_back(future.get());
#else
        for (auto& chunk : _chunks)
            meshes.push_back(chunk.generate_mesh());
#endif
        generate_chunk_meshes_time = timer.elapsed_ms();
    }

    double upload_chunk_meshes_time;

    {
        zth::Timer timer;

        usize i = 0;

        for (auto& chunk : _chunks)
            chunk.upload_mesh(chunk_meshes[i++]);

        upload_chunk_meshes_time = timer.elapsed_ms();
    }

    double create_chunk_entities_time;

    {
        zth::Timer timer;

        usize chunk_index = 0;

        for (auto [x, z] : coords)
            create_chunk_entity(_chunks[chunk_index++], x, z);

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

auto MainScene::create_chunk_entity(const Chunk& chunk, i32 chunk_x, i32 chunk_z) -> void
{
    auto entity = create_entity("Chunk");

    auto world_position = glm::vec3{ Chunk::to_world_x(chunk_x), 0.0f, Chunk::to_world_z(chunk_z) };
    entity.transform().set_translation(world_position);

    entity.emplace_or_replace<zth::MeshComponent>(&chunk.mesh());
    entity.emplace_or_replace<zth::MaterialComponent>(&_block_material);
}
