#pragma once

class MainScene : public zth::Scene
{
public:
    explicit MainScene() = default;
    ZTH_NO_COPY_NO_MOVE(MainScene)
    ~MainScene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");
    zth::EntityHandle _directional_light = create_entity("Directional Light");

private:
    auto on_load() -> void override;
};
