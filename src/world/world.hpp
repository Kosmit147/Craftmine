#pragma once

#include "chunk.hpp"

class World
{
public:
    World() = delete;

    static auto init(zth::Scene& scene) -> void;
    static auto shut_down() -> void;

private:
    static std::unordered_map<ChunkPosition, Chunk> _world;

    static std::shared_ptr<zth::gl::Texture2D> _blocks_texture;
    static std::shared_ptr<zth::Material> _chunk_material;
    static zth::Scene* _scene;

private:
    static auto generate(i32 x_chunks, i32 z_chunks) -> void;
    static auto create_chunk_entity(const Chunk& chunk, ChunkPosition position) -> void;
};
