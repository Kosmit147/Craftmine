#include "player.hpp"

namespace scripts {

auto Player::display_label() const -> const char*
{
    return "Player";
}

auto Player::debug_edit() -> void
{
    zth::debug::drag_float("Movement Speed", movement_speed);
    zth::debug::drag_float("Mouse Sensitivity", mouse_sensitivity);

    zth::debug::select_key("Move Forward Key", move_forward_key);
    zth::debug::select_key("Move Backward Key", move_backward_key);
    zth::debug::select_key("Move Left Key", move_left_key);
    zth::debug::select_key("Move Right Key", move_right_key);

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

auto Player::on_update(zth::EntityHandle actor) -> void
{
    auto& transform = actor.transform();

    {
        // Move around.

        auto speed = movement_speed * zth::Time::delta_time<float>();

        if (sprinting_enabled)
        {
            if (zth::Input::is_key_pressed(sprint_key))
                speed *= sprinting_speed_multiplier;
        }

        auto forward = transform.forward() * speed;
        auto backward = -forward;
        auto right = transform.right() * speed;
        auto left = -right;

        if (zth::Input::is_key_pressed(move_forward_key))
            transform.translate(forward);

        if (zth::Input::is_key_pressed(move_backward_key))
            transform.translate(backward);

        if (zth::Input::is_key_pressed(move_right_key))
            transform.translate(right);

        if (zth::Input::is_key_pressed(move_left_key))
            transform.translate(left);
    }

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
}

} // namespace scripts
