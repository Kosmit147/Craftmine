#pragma once

class MenuScene : public zth::Scene
{
public:
    constexpr static glm::vec4 sky_color = zth::normalize_rgba8(110, 177, 255, 255);

public:
    explicit MenuScene() = default;
    ZTH_NO_COPY_NO_MOVE(MenuScene)
    ~MenuScene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");

    zth::EntityHandle _directional_light = create_entity("Directional Light");
    zth::EntityHandle _world_manager = create_entity("World Manager");

    zth::EntityHandle _logo = create_entity("Logo");
    zth::EntityHandle _start_button = create_entity("Start Button");
    zth::EntityHandle _exit_button = create_entity("Exit Button");

    std::shared_ptr<zth::gl::Texture2D> _exit_button_texture;
    std::shared_ptr<zth::gl::Texture2D> _exit_button_hover_texture;
    std::shared_ptr<zth::gl::Texture2D> _logo_texture;
    // std::shared_ptr<zth::gl::Texture2D> _menu_background_texture;
    std::shared_ptr<zth::gl::Texture2D> _start_button_texture;
    std::shared_ptr<zth::gl::Texture2D> _start_button_hover_texture;

    glm::vec2 _last_cursor_pos{ 0.0f };

private:
    auto on_event(const zth::Event& event) -> void override;
    auto on_load() -> void override;

    auto update_ui(glm::vec2 mouse_pos = glm::vec2{ 0.0f }, bool clicked = false) -> void;
};
