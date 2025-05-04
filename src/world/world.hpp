#pragma once

#include <future>
#include <queue>

#include "chunk.hpp"

// Multithreaded chunk generation
//
// We're generating every chunk by performing these 4 steps in order:
// 1. Generating the blocks procedurally.
// 2. Generating a GPU mesh for the chunk.
// 3. Uploading the chunk mesh to the GPU.
// 4. Creating a chunk entity with a mesh component.
//
// Steps 1 and 2 can safely be performed on another thread. On the other hand, uploading the chunk's mesh has to be done
// on the thread with the current OpenGL context, which is the main thread. Creating entities is not computationally
// costly at all, so it doesn't seem worth it to make that multithreaded.
//
// During step 2, the mesh generation function needs to access neighboring chunks to determine whether the outer faces
// of the chunk's blocks are visible. This means that during that time the write access to the chunk map needs to be
// locked to the thread that performs the mesh generation, because adding a chunk on another thread could cause a
// rehash, which would cause the references to chunks in that map to become invalid.

struct NewChunkParams
{
    ChunkPosition position;
    ChunkData chunk_data;
    zth::Vector<zth::StandardVertex> mesh;
};

class World
{
public:
    static const usize max_generate_chunks_tasks;
    static const usize max_chunks_to_create_each_frame;

public:
    World() = delete;

    static auto init(i32 distance) -> void;
    static auto update(zth::Scene& scene) -> void;
    static auto shut_down() -> void;

private:
    static std::unordered_map<ChunkPosition, Chunk> _map;

    static std::queue<ChunkPosition> _generate_chunk_requests;
    static std::queue<std::future<NewChunkParams>> _generate_chunk_tasks;

    static std::shared_ptr<zth::gl::Texture2D> _blocks_texture;
    static std::shared_ptr<zth::Material> _chunk_material;

    static zth::Timer _generate_chunks_timer;
    static inline bool _all_chunks_generated = false;

private:
    static auto generate_new_chunk(ChunkPosition position) -> NewChunkParams;
    static auto create_chunk_entity(zth::Scene& scene, const Chunk& chunk, ChunkPosition position) -> void;
};
