#include "main_scene.hpp"

MainScene::MainScene()
{
    _camera.emplace<zth::CameraComponent>(zth::Window::aspect_ratio());
    _camera.emplace<zth::ScriptComponent>(std::make_unique<zth::scripts::FlyCamera>());

    _directional_light.emplace<zth::LightComponent>(zth::DirectionalLight{});
    _directional_light.transform().set_direction(glm::normalize(glm::vec3{ -0.35f, -1.0f, -0.35f }));

    _camera.transform().set_translation(glm::vec3{ 0.0f, 90.0f, 5.0f });

    ChunkGenerator::init(1234);
    generate_world(3, 3);
}

auto MainScene::generate_world(u32 x_chunks, u32 z_chunks) -> void
{
    for (u32 x = 0; x < x_chunks; x++)
    {
        for (u32 z = 0; z < z_chunks; z++)
        {
            auto& chunk = _chunks.emplace_back(ChunkGenerator::generate(x, z));
            generate_entities(*chunk, x, z);
        }
    }
}

auto MainScene::generate_entities(Chunk& chunk, u32 chunk_x, u32 chunk_z) -> void
{
    auto& chunk_size = Chunk::size;

    for (u32 x = 0; x < chunk_size.x; ++x)
    {
        for (u32 z = 0; z < chunk_size.z; ++z)
        {
            for (u32 y = 0; y < chunk_size.y; ++y)
            {
                auto view = chunk.view_3d();

                BlockType type = view[x, y, z];

                if (type == BlockType::Air)
                    continue;

                zth::EntityHandle cube = create_entity("Cube");

                cube.emplace<zth::MeshComponent>(&zth::meshes::cube_mesh());
                cube.emplace<zth::MaterialComponent>(&_cube_material);

                cube.transform().set_translation(glm::vec3{ static_cast<float>(chunk_x * chunk_size.x + x),
                                                            static_cast<float>(y),
                                                            static_cast<float>(chunk_z * chunk_size.z + z) });
            }
        }
    }
}
