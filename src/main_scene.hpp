#pragma once

class MainScene : public zth::Scene
{
public:
    explicit MainScene();
    ZTH_NO_COPY_NO_MOVE(MainScene)
    ~MainScene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");
    zth::EntityHandle _directional_light = create_entity("Directional Light");
    zth::EntityHandle _cube = create_entity("Cube");

    zth::Material _cube_material;

private:
    auto on_event(const zth::Event& event) -> void override;
};
