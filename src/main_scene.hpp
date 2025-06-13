/**
 * @file main_scene.hpp
 * @brief Defines the MainScene class, which represents the main gameplay scene.
 */

#pragma once

/**
 * @class MainScene
 * @brief Represents the main gameplay scene of the application.
 *
 * This scene handles the player, world manager, lighting, and UI elements such as the crosshair.
 */
class MainScene : public zth::Scene
{
public:
    /**
     * @brief The color of the sky in the main scene.
     */
    constexpr static glm::vec4 sky_color = zth::normalize_rgba8(110, 177, 255, 255);

public:
    /**
     * @brief Constructs a MainScene object.
     */
    explicit MainScene() = default;

    /**
     * @brief Deleted copy constructor and move constructor.
     */
    ZTH_NO_COPY_NO_MOVE(MainScene)

    /**
     * @brief Destructor for MainScene.
     */
    ~MainScene() override = default;

private:
    /**
     * @brief The player entity handle.
     */
    zth::EntityHandle _player = create_entity("Player");

    /**
     * @brief The directional light entity handle.
     */
    zth::EntityHandle _directional_light = create_entity("Directional Light");

    /**
     * @brief The world manager entity handle.
     */
    zth::EntityHandle _world_manager = create_entity("World Manager");

    /**
     * @brief The crosshair entity handle.
     */
    zth::EntityHandle _crosshair = create_entity("Crosshair");

    /**
     * @brief The crosshair texture.
     */
    std::shared_ptr<zth::gl::Texture2D> _crosshair_texture;

private:
    /**
     * @brief Handles events for the main scene.
     * @param event The event to handle.
     */
    auto on_event(const zth::Event& event) -> void override;

    /**
     * @brief Called when the scene is loaded.
     */
    auto on_load() -> void override;

    /**
     * @brief Updates the UI elements of the scene.
     */
    auto update_ui() -> void;
};
