#pragma once

#include "world/chunk.hpp"

class MainScene : public zth::Scene
{
public:
    explicit MainScene() = default;
    ZTH_NO_COPY_NO_MOVE(MainScene)
    ~MainScene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");
    zth::EntityHandle _directional_light = create_entity("Directional Light");

    zth::Material _block_material;
    zth::Vector<Chunk> _chunks;

private:
    auto on_load() -> void override;

    auto generate_world(i32 x_chunks, i32 z_chunks) -> void;
    auto create_chunk_entity(const Chunk& chunk, i32 chunk_x, i32 chunk_z) -> void;
};
