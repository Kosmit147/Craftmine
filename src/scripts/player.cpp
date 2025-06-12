#include "player.hpp"

namespace scripts {

auto Player::display_label() const -> const char*
{
    return "Player";
}

auto Player::debug_edit() -> void
{
    zth::debug::drag_float("Movement Speed", movement_speed);
    zth::debug::drag_float("Jump Speed", jump_speed);
    zth::debug::drag_float("Mouse Sensitivity", mouse_sensitivity);

    zth::debug::select_key("Move Forward Key", move_forward_key);
    zth::debug::select_key("Move Backward Key", move_backward_key);
    zth::debug::select_key("Move Left Key", move_left_key);
    zth::debug::select_key("Move Right Key", move_right_key);
    zth::debug::select_key("Jump Key", jump_key);

    zth::debug::checkbox("Sprinting Enabled", sprinting_enabled);

    if (sprinting_enabled)
    {
        zth::debug::select_key("Sprint Key", sprint_key);
        zth::debug::drag_float("Sprinting Speed Multiplier", sprinting_speed_multiplier);
    }
}

void Player::on_event(zth::EntityHandle actor, const zth::Event& event)
{
    if (event.type() == zth::EventType::WindowResized)
    {
        auto [new_size] = event.window_resized_event();

        if (!actor.any_of<zth::CameraComponent>())
            return;

        auto& camera = actor.get<zth::CameraComponent>();
        camera.aspect_ratio = static_cast<float>(new_size.x) / static_cast<float>(new_size.y);
    }
}

auto Player::on_fixed_update(zth::EntityHandle actor) -> void
{
    auto& transform = actor.transform();

    if (!zth::Window::cursor_enabled())
    {
        // Look around.

        auto mouse_delta =
            zth::Input::mouse_pos_delta() / static_cast<float>(zth::Window::size().y) * mouse_sensitivity;
        auto angles = transform.euler_angles();

        angles.pitch -= mouse_delta.y;
        angles.yaw -= mouse_delta.x;
        angles.roll = 0.0f;

        angles.pitch = std::clamp(angles.pitch, min_pitch, max_pitch);

        transform.set_rotation(angles);
    }

    if (!actor.any_of<zth::CharacterControllerComponent>())
        return;

    auto movement_direction = JPH::Vec3::sZero();

    {
        // Move around.

        auto raw_forward = transform.forward();

        auto forward = glm::normalize(glm::vec3{ raw_forward.x, 0.0f, raw_forward.z });
        auto backward = -forward;
        auto right = glm::normalize(glm::cross(forward, zth::math::world_up));
        auto left = -right;

        if (zth::Input::is_key_pressed(move_forward_key))
            movement_direction += JPH::Vec3{ forward.x, forward.y, forward.z };

        if (zth::Input::is_key_pressed(move_backward_key))
            movement_direction += JPH::Vec3{ backward.x, backward.y, backward.z };

        if (zth::Input::is_key_pressed(move_right_key))
            movement_direction += JPH::Vec3{ right.x, right.y, right.z };

        if (zth::Input::is_key_pressed(move_left_key))
            movement_direction += JPH::Vec3{ left.x, left.y, left.z };
    }

    auto& controller = actor.get<zth::CharacterControllerComponent>();
    auto& character = controller.character;

    auto desired_velocity = movement_direction * movement_speed;

    if (sprinting_enabled && zth::Input::is_key_pressed(sprint_key))
        desired_velocity *= sprinting_speed_multiplier;

    // True if the player intended to move
    // mAllowSliding = !inMovementDirection.IsNearZero();

    // @todo: The rotation is wrong!
    // Up should always be (0, 1, 0).

    // auto transform_rotation = transform.rotation();
    // JPH::Quat rotation{ transform_rotation.x, transform_rotation.y, transform_rotation.z, transform_rotation.w };
    // JPH::Quat character_up_rotation{ rotation.x, rotation.y, rotation.z, rotation.w };
    // character->SetUp(character_up_rotation.RotateAxisY());
    character->SetUp(JPH::Vec3::sAxisY());
    character->SetRotation(JPH::Quat::sIdentity());

    // @todo: Do we need to call this?
    character->UpdateGroundVelocity();

    // Determine new basic velocity
    auto current_vertical_velocity = character->GetLinearVelocity().Dot(character->GetUp()) * character->GetUp();
    auto ground_velocity = character->GetGroundVelocity();
    JPH::Vec3 new_velocity;
    auto moving_towards_ground = (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.1f;

    if (character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround
        && !character->IsSlopeTooSteep(character->GetGroundNormal()))
    {
        // Assume velocity of ground when on ground
        new_velocity = ground_velocity;

        // Jump
        if (zth::Input::is_key_pressed(jump_key) && moving_towards_ground)
            new_velocity += jump_speed * character->GetUp();
    }
    else
    {
        new_velocity = current_vertical_velocity;
    }

    // Gravity
    new_velocity += zth::Physics::gravity() * zth::Time::fixed_time_step<float>();

    // Player input
    new_velocity += desired_velocity;

    // Update character velocity
    character->SetLinearVelocity(new_velocity);
}

auto Player::on_attach(zth::EntityHandle actor) -> void
{
    actor.try_emplace<zth::CharacterControllerComponent>();
}

} // namespace scripts
