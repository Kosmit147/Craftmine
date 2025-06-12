#pragma once

class MainScene : public zth::Scene
{
public:
    constexpr static glm::vec4 sky_color = zth::normalize_rgba8(110, 177, 255, 255);

public:
    explicit MainScene() = default;
    ZTH_NO_COPY_NO_MOVE(MainScene)
    ~MainScene() override = default;

private:
    zth::EntityHandle _player = create_entity("Player");

    zth::EntityHandle _directional_light = create_entity("Directional Light");
    zth::EntityHandle _world_manager = create_entity("World Manager");

    zth::EntityHandle _crosshair = create_entity("Crosshair");

    std::shared_ptr<zth::gl::Texture2D> _crosshair_texture;

private:
    auto on_event(const zth::Event& event) -> void override;
    auto on_load() -> void override;

    auto update_ui() -> void;
};
