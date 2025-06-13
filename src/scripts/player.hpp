/**
 * @file player.hpp
 * @brief Declares the Player script class for player control and behavior.
 */

#pragma once

namespace scripts {

/**
 * @class Player
 * @brief Script class for controlling the player character.
 *
 * Handles movement, jumping, sprinting, mouse look, and input bindings.
 */
class Player : public zth::Script
{
public:
    /**
     * @brief Minimum pitch angle for camera rotation (in radians).
     */
    static constexpr auto min_pitch = glm::radians(-89.0f);

    /**
     * @brief Maximum pitch angle for camera rotation (in radians).
     */
    static constexpr auto max_pitch = glm::radians(89.0f);

    /**
     * @brief Player movement speed (units per second).
     */
    float movement_speed = 5.0f;

    /**
     * @brief Player jump speed.
     */
    float jump_speed = 5.0f;

    /**
     * @brief Mouse sensitivity for camera rotation.
     */
    float mouse_sensitivity = 1.4f;

    /**
     * @brief Key binding for moving forward.
     */
    zth::Key move_forward_key = zth::Key::W;

    /**
     * @brief Key binding for moving backward.
     */
    zth::Key move_backward_key = zth::Key::S;

    /**
     * @brief Key binding for moving left.
     */
    zth::Key move_left_key = zth::Key::A;

    /**
     * @brief Key binding for moving right.
     */
    zth::Key move_right_key = zth::Key::D;

    /**
     * @brief Key binding for jumping.
     */
    zth::Key jump_key = zth::Key::Space;

    /**
     * @brief Enables or disables sprinting.
     */
    bool sprinting_enabled = true;

    /**
     * @brief Key binding for sprinting.
     */
    zth::Key sprint_key = zth::Key::LeftShift;

    /**
     * @brief Speed multiplier when sprinting.
     */
    float sprinting_speed_multiplier = 3.0f;

public:
    /**
     * @brief Default constructor.
     */
    Player() = default;

    /**
     * @brief Default copy and move constructors and assignment operators.
     */
    ZTH_DEFAULT_COPY_DEFAULT_MOVE(Player)

    /**
     * @brief Destructor.
     */
    ~Player() override = default;

    /**
     * @brief Returns a label for display in debug or editor UI.
     * @return The display label as a C-string.
     */
    [[nodiscard]] auto display_label() const -> const char* override;

    /**
     * @brief Allows editing of player parameters in a debug UI.
     */
    auto debug_edit() -> void override;

    /**
     * @brief Handles events for the player script.
     * @param actor The entity handle for the player.
     * @param event The event to process.
     */
    auto on_event(zth::EntityHandle actor, const zth::Event& event) -> void override;

    /**
     * @brief Called on each fixed update tick for physics and movement.
     * @param actor The entity handle for the player.
     */
    auto on_fixed_update(zth::EntityHandle actor) -> void override;

private:
    /**
     * @brief Called when the script is attached to an entity.
     * @param actor The entity handle to which the script is attached.
     */
    auto on_attach(zth::EntityHandle actor) -> void override;

};

} // namespace scripts
