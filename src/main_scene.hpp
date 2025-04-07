#pragma once

#include "world/chunk.hpp"

class MainScene : public zth::Scene
{
public:
    explicit MainScene();
    ZTH_NO_COPY_NO_MOVE(MainScene)
    ~MainScene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");
    zth::EntityHandle _directional_light = create_entity("Directional Light");
    zth::Material _cube_material;
    zth::Material _grass_material;
    zth::Material _dirt_material;
    zth::Material _stone_material;

    zth::Vector<std::unique_ptr<Chunk>> _chunks;

private:
    auto generate_world(u32 x_chunks, u32 z_chunks) -> void;
    auto generate_entities(Chunk& chunk, u32 chunk_x, u32 chunk_z) -> void;
};
