#include "main_scene.hpp"

#include "assets.hpp"
#include "scripts/player.hpp"
#include "scripts/world_manager.hpp"

using namespace zth::hashed_string_literals;

auto MainScene::on_event(const zth::Event& event) -> void
{
    if (event.type() == zth::EventType::WindowResized)
        update_ui();
}

auto MainScene::on_load() -> void
{
    zth::Window::set_cursor_enabled(false);
    zth::Renderer::set_clear_color(sky_color);

    _player.emplace_or_replace<zth::CameraComponent>(zth::CameraComponent{
        .aspect_ratio = zth::Window::aspect_ratio(),
        .fov = glm::radians(45.0f),
        .near = 0.1f,
        .far = 1000.0f,
    });

    _player.transform().scale(2.0f);
    _player.emplace_or_replace<zth::ScriptComponent>(zth::make_unique<scripts::Player>());
    _player.transform().set_translation(glm::vec3{ 0.0f, 120.0f, 0.0f }).set_direction(zth::math::world_backward);

    _directional_light.emplace_or_replace<zth::LightComponent>(zth::DirectionalLight{});
    _directional_light.transform().set_direction(glm::normalize(glm::vec3{ -0.35f, -1.0f, -0.35f }));

    _world_manager.emplace_or_replace<zth::ScriptComponent>(zth::make_unique<scripts::WorldManager>(_player));

    _crosshair_texture =
        zth::AssetManager::emplace<zth::gl::Texture2D>(
            "crosshair_texture"_hs,
            zth::gl::Texture2D::from_file_data(
                crosshair_texture_data, zth::gl::TextureParams{ .mag_filter = zth::gl::TextureMagFilter::nearest }))
            ->get();

    update_ui();
}

auto MainScene::update_ui() -> void
{
    _crosshair.emplace_or_replace<zth::SpriteRenderer2DComponent>(
        zth::textures::white(), zth::Rect<zth::u32>{
                                    .position = glm::ivec2{ zth::Window::size() } / 2 + glm::ivec2{ -2, 2 },
                                    .size = glm::uvec2{ 4u, 4u },
                                });
}
