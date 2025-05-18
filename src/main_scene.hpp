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

private:
    auto on_load() -> void override;
};
