#pragma once

class Scene : public zth::Scene
{
public:
    explicit Scene();
    ZTH_NO_COPY_NO_MOVE(Scene)
    ~Scene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");
    zth::EntityHandle _directional_light = create_entity("Directional Light");
    zth::EntityHandle _cube = create_entity("Cube");

    zth::Material _cube_material;

private:
    auto on_event(const zth::Event& event) -> void override;
};
