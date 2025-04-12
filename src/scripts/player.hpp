#pragma once

namespace scripts {

class Player : public zth::Script
{
public:
    static constexpr auto min_pitch = glm::radians(-89.0f);
    static constexpr auto max_pitch = glm::radians(89.0f);

    float movement_speed = 10.0f;
    float mouse_sensitivity = 1.4f;

    zth::Key move_forward_key = zth::Key::W;
    zth::Key move_backward_key = zth::Key::S;
    zth::Key move_left_key = zth::Key::A;
    zth::Key move_right_key = zth::Key::D;

    bool sprinting_enabled = true;
    zth::Key sprint_key = zth::Key::LeftShift;
    float sprinting_speed_multiplier = 3.0f;

public:
    Player() = default;
    ZTH_DEFAULT_COPY_DEFAULT_MOVE(Player)
    ~Player() override = default;

    [[nodiscard]] auto display_label() const -> const char* override;
    auto debug_edit() -> void override;

private:
    auto on_event(zth::EntityHandle actor, const zth::Event& event) -> void override;
    auto on_update(zth::EntityHandle actor) -> void override;
};

} // namespace scripts
