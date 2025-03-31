#include "scene.hpp"

Scene::Scene()
{
    _camera.emplace<zth::CameraComponent>(zth::Window::aspect_ratio());
    _camera.emplace<zth::ScriptComponent>(std::make_unique<zth::scripts::FlyCamera>());

    _directional_light.emplace<zth::LightComponent>(zth::DirectionalLight{});

    _cube.emplace<zth::MeshComponent>(&zth::meshes::cube_mesh());
    _cube.emplace<zth::MaterialComponent>(&_cube_material);

    auto& camera_transform = _camera.get<zth::TransformComponent>();
    auto& directional_light_transform = _directional_light.get<zth::TransformComponent>();

    camera_transform.set_translation(glm::vec3{ 0.0f, 0.0f, 5.0f });
    directional_light_transform.set_direction(glm::vec3{ -0.35f, -1.0f, -0.35 });
}

auto Scene::on_event(const zth::Event& event) -> void
{
    if (event.type() == zth::EventType::KeyPressed)
    {
        auto [key] = event.key_pressed_event();

        if (key == zth::Key::Escape)
            zth::Window::close();
        else if (key == zth::Key::LeftControl)
            zth::Window::set_cursor_enabled(!zth::Window::cursor_enabled());
    }
    else if (event.type() == zth::EventType::WindowResized)
    {
        auto [new_size] = event.window_resized_event();

        auto& camera = _camera.get<zth::CameraComponent>();
        camera.aspect_ratio = static_cast<float>(new_size.x) / static_cast<float>(new_size.y);
    }
}
