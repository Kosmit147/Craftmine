#include "menu_scene.hpp"

#include "assets.hpp"
#include "main_scene.hpp"
#include "scripts/world_manager.hpp"

using namespace zth::hashed_string_literals;

namespace {

auto mouse_inside_rect(glm::uvec2 mouse, glm::uvec2 window_size, const zth::Rect<zth::u32>& rect) -> bool
{
    mouse = glm::uvec2{ mouse.x, window_size.y - mouse.y }; // Flip y on the mouse.

    auto top_left = rect.top_left();
    auto bottom_right = rect.bottom_right();

    return mouse.x > top_left.x && mouse.x < bottom_right.x && mouse.y > bottom_right.y && mouse.y < top_left.y;
}

} // namespace

auto MenuScene::on_event(const zth::Event& event) -> void
{
    if (event.type() == zth::EventType::WindowResized)
    {
        update_ui();
    }
    else if (event.type() == zth::EventType::MouseMoved)
    {
        auto ev = event.mouse_moved_event();
        update_ui(ev.new_pos);
        _last_cursor_pos = ev.new_pos;
    }
    else if (event.type() == zth::EventType::MouseButtonPressed)
    {
        update_ui(_last_cursor_pos, true);
    }
}

auto MenuScene::on_load() -> void
{
    zth::Window::set_cursor_enabled(true);
    zth::Renderer::set_clear_color(zth::colors::black);

    _camera.emplace_or_replace<zth::CameraComponent>(zth::CameraComponent{
        .aspect_ratio = zth::Window::aspect_ratio(),
        .fov = glm::radians(45.0f),
        .near = 0.1f,
        .far = 1000.0f,
    });

    _directional_light.emplace_or_replace<zth::LightComponent>(zth::DirectionalLight{});
    _directional_light.transform().set_direction(glm::normalize(glm::vec3{ -0.35f, -1.0f, -0.35f }));

    _exit_button_texture = zth::AssetManager::emplace<zth::gl::Texture2D>(
                               "exit_button_texture"_hs, zth::gl::Texture2D::from_file_data(exit_button_texture_data))
                               ->get();

    _exit_button_hover_texture =
        zth::AssetManager::emplace<zth::gl::Texture2D>(
            "exit_button_hover_texture"_hs, zth::gl::Texture2D::from_file_data(exit_button_hover_texture_data))
            ->get();

    _logo_texture = zth::AssetManager::emplace<zth::gl::Texture2D>(
                        "logo_texture"_hs, zth::gl::Texture2D::from_file_data(logo_texture_data))
                        ->get();

    _start_button_texture =
        zth::AssetManager::emplace<zth::gl::Texture2D>("start_button_texture"_hs,
                                                       zth::gl::Texture2D::from_file_data(start_button_texture_data))
            ->get();

    _start_button_hover_texture =
        zth::AssetManager::emplace<zth::gl::Texture2D>(
            "start_button_hover_texture"_hs, zth::gl::Texture2D::from_file_data(start_button_hover_texture_data))
            ->get();

    _logo.emplace<zth::SpriteRenderer2DComponent>();
    _start_button.emplace<zth::SpriteRenderer2DComponent>();
    _exit_button.emplace<zth::SpriteRenderer2DComponent>();

    update_ui();
}

auto MenuScene::update_ui(glm::vec2 mouse_pos, bool clicked) -> void
{
    ZTH_INFO("Mouse pos: {}", mouse_pos);
    glm::uvec2 umouse_pos{ mouse_pos };
    auto window_size = zth::Window::size();

    zth::Rect<zth::u32> logo_rect{
        .position = glm::ivec2{ zth::Window::size() } / 2 + glm::ivec2{ -300, 50 + 300 },
        .size = glm::uvec2{ 600u, 100u },
    };

    zth::Rect<zth::u32> start_button_rect{
        .position = glm::ivec2{ zth::Window::size() } / 2 + glm::ivec2{ -200, 20 },
        .size = glm::uvec2{ 400u, 40u },
    };

    zth::Rect<zth::u32> exit_button_rect{
        .position = glm::ivec2{ zth::Window::size() } / 2 + glm::ivec2{ -200, 20 - 50 },
        .size = glm::uvec2{ 400u, 40u },
    };

    auto* start_button_texture = &_start_button_texture;
    auto* exit_button_texture = &_exit_button_texture;

    if (mouse_inside_rect(umouse_pos, window_size, start_button_rect))
    {
        start_button_texture = &_start_button_hover_texture;

        if (clicked)
            zth::SceneManager::queue_scene<MainScene>();
    }

    if (mouse_inside_rect(umouse_pos, window_size, exit_button_rect))
    {
        exit_button_texture = &_exit_button_hover_texture;

        if (clicked)
            zth::Window::close();
    }

    _logo.patch<zth::SpriteRenderer2DComponent>([&](auto& logo) {
        logo.texture = _logo_texture;
        logo.rect = logo_rect;
    });

    _start_button.patch<zth::SpriteRenderer2DComponent>([&](auto& start_button) {
        start_button.texture = *start_button_texture;
        start_button.rect = start_button_rect;
    });

    _exit_button.patch<zth::SpriteRenderer2DComponent>([&](auto& exit_button) {
        exit_button.texture = *exit_button_texture;
        exit_button.rect = exit_button_rect;
    });
}
