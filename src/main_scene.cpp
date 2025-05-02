#include "main_scene.hpp"

#include "scripts/player.hpp"
#include "world/world.hpp"

auto MainScene::on_load() -> void
{
    zth::Renderer::set_clear_color(sky_color);

    _player.emplace<zth::CameraComponent>(zth::CameraComponent{
        .aspect_ratio = zth::Window::aspect_ratio(),
        .fov = glm::radians(45.0f),
        .near = 0.1f,
        .far = 1000.0f,
    });

    _player.emplace<zth::ScriptComponent>(zth::make_unique<scripts::Player>());

    _directional_light.emplace<zth::LightComponent>(zth::DirectionalLight{});
    _directional_light.transform().set_direction(glm::normalize(glm::vec3{ -0.35f, -1.0f, -0.35f }));

    _player.transform().set_translation(glm::vec3{ 0.0f, 120.0f, 0.0f });
    _player.transform().set_direction(zth::math::world_backward);

    World::init(*this);
}
