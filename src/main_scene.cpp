#include "main_scene.hpp"


MainScene::MainScene()
{
    _camera.emplace<zth::CameraComponent>(zth::Window::aspect_ratio());
    _camera.emplace<zth::ScriptComponent>(std::make_unique<zth::scripts::FlyCamera>());

    _directional_light.emplace<zth::LightComponent>(zth::DirectionalLight{});
    generate_world(3, 3);


    auto& camera_transform = _camera.get<zth::TransformComponent>();
    auto& directional_light_transform = _directional_light.get<zth::TransformComponent>();

    camera_transform.set_translation(glm::vec3{ 0.0f, 90.0f, 5.0f });
    directional_light_transform.set_direction(glm::vec3{ -0.35f, -1.0f, -0.35 });
}

auto MainScene::on_event(const zth::Event& event) -> void
{
    if (event.type() == zth::EventType::WindowResized)
    {
        auto [new_size] = event.window_resized_event();

        auto& camera = _camera.get<zth::CameraComponent>();
        camera.aspect_ratio = static_cast<float>(new_size.x) / static_cast<float>(new_size.y);
    }
}

void MainScene::generate_world(int worldWidth, int worldDepth)
{
    ChunkGenerator generator(1234);

    for (int chunkX = 0; chunkX < worldWidth; ++chunkX)
    {
        for (int chunkZ = 0; chunkZ < worldDepth; ++chunkZ)
        {
            ChunkData chunk = generator.generate_chunk(chunkX, chunkZ);

            generate_entities(chunk,chunkX,chunkZ);
        }
    }
}

void MainScene::generate_entities(ChunkData& chunk, int chunkX, int chunkZ)
{
    for (int x = 0; x < CHUNK_WIDTH; ++x)
    {
        for (int z = 0; z < CHUNK_DEPTH; ++z)
        {
            for (int y = 0; y < CHUNK_HEIGHT; ++y)
            {
                BlockType type = chunk[x][z][y];
                if (type == BlockType::Air)
                    continue;

                zth::EntityHandle cube = create_entity("Cube");

                cube.emplace<zth::MeshComponent>(&zth::meshes::cube_mesh());
                cube.emplace<zth::MaterialComponent>(&_cube_material);

                auto& transform = cube.get<zth::TransformComponent>();
                transform.set_translation(glm::vec3{
                    static_cast<float>(chunkX * CHUNK_WIDTH + x),
                    static_cast<float>(y),                       
                    static_cast<float>(chunkZ * CHUNK_DEPTH + z) 
                });
            }
        }
    }
}
