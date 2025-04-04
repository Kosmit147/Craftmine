#pragma once
#include "world_generator/chunk_generator.hpp"
class MainScene : public zth::Scene
{
public:
    explicit MainScene();
    void generate_entities(ChunkData& chunk, int chunkX, int chunkZ);
    void generate_world(int worldWidth, int worldDepth);
    ZTH_NO_COPY_NO_MOVE(MainScene)
    ~MainScene() override = default;

private:
    zth::EntityHandle _camera = create_entity("Camera");
    zth::EntityHandle _directional_light = create_entity("Directional Light");
    zth::EntityHandle _cube = create_entity("Cube");
    zth::Material _cube_material;
    zth::Material _grass_material;
    zth::Material _dirt_material;
    zth::Material _stone_material;
    

private:
    auto on_event(const zth::Event& event) -> void override;
};
