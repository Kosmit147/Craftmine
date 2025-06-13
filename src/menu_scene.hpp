/**
 * @file menu_scene.hpp
 * @brief Declares the MenuScene class, which manages the main menu scene and its UI elements.
 */

#pragma once

/**
 * @class MenuScene
 * @brief Represents the main menu scene of the application.
 *
 * Handles the menu UI, including camera, lighting, logo, and interactive buttons.
 */
class MenuScene : public zth::Scene
{
public:
    /**
     * @brief The color of the sky in the menu scene.
     */
    constexpr static glm::vec4 sky_color = zth::normalize_rgba8(110, 177, 255, 255);

public:
    /**
     * @brief Constructs a MenuScene object.
     */
    explicit MenuScene() = default;

    /**
     * @brief Deleted copy constructor and move constructor.
     */
    ZTH_NO_COPY_NO_MOVE(MenuScene)

    /**
     * @brief Destructor for MenuScene.
     */
    ~MenuScene() override = default;

private:
    /**
     * @brief The camera entity handle for the menu scene.
     */
    zth::EntityHandle _camera = create_entity("Camera");

    /**
     * @brief The directional light entity handle.
     */
    zth::EntityHandle _directional_light = create_entity("Directional Light");

    /**
     * @brief The world manager entity handle (not always used in menu).
     */
    zth::EntityHandle _world_manager = create_entity("World Manager");

    /**
     * @brief The logo entity handle.
     */
    zth::EntityHandle _logo = create_entity("Logo");

    /**
     * @brief The start button entity handle.
     */
    zth::EntityHandle _start_button = create_entity("Start Button");

    /**
     * @brief The exit button entity handle.
     */
    zth::EntityHandle _exit_button = create_entity("Exit Button");

    /**
     * @brief Texture for the exit button (normal state).
     */
    std::shared_ptr<zth::gl::Texture2D> _exit_button_texture;

    /**
     * @brief Texture for the exit button (hover state).
     */
    std::shared_ptr<zth::gl::Texture2D> _exit_button_hover_texture;

    /**
     * @brief Texture for the logo.
     */
    std::shared_ptr<zth::gl::Texture2D> _logo_texture;

    // std::shared_ptr<zth::gl::Texture2D> _menu_background_texture;

    /**
     * @brief Texture for the start button (normal state).
     */
    std::shared_ptr<zth::gl::Texture2D> _start_button_texture;

    /**
     * @brief Texture for the start button (hover state).
     */
    std::shared_ptr<zth::gl::Texture2D> _start_button_hover_texture;

    /**
     * @brief Last known cursor position, used for UI interaction.
     */
    glm::vec2 _last_cursor_pos{ 0.0f };

private:
    /**
     * @brief Handles events for the menu scene.
     * @param event The event to handle.
     */
    auto on_event(const zth::Event& event) -> void override;

    /**
     * @brief Called when the menu scene is loaded.
     */
    auto on_load() -> void override;

    /**
     * @brief Updates the UI elements of the menu scene.
     * @param mouse_pos The current mouse position (default is {0,0}).
     * @param clicked Whether the mouse button was clicked (default is false).
     */
    auto update_ui(glm::vec2 mouse_pos = glm::vec2{ 0.0f }, bool clicked = false) -> void;
};
