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
// of the chunk's exterior blocks are visible.

struct GenerateChunkResults
{
    ChunkPosition position;
    std::shared_ptr<ChunkData> chunk_data;
};

struct UpdateChunkMeshInputs
{
    std::shared_ptr<const ChunkData> chunk_data;
    NeighboringChunksData neighbors;
};

struct UpdateChunkMeshResults
{
    ChunkPosition position;
    ChunkMesh mesh;
};

class World
{
public:
    static const usize max_generate_chunk_tasks;
    static const usize max_update_chunk_mesh_tasks;

public:
    World() = delete;

    static auto init(i32 distance) -> void;
    static auto update(zth::Scene& scene) -> void;
    static auto shut_down() -> void;

private:
    static std::unordered_map<ChunkPosition, Chunk> _map;
    static std::unordered_map<ChunkPosition, zth::EntityHandle> _entity_map;

    // These are the requests to generate a new chunk from scratch.
    static std::queue<ChunkPosition> _generate_chunk_requests;
    static std::queue<std::future<GenerateChunkResults>> _generate_chunk_tasks;

    // These are the requests to update a chunk's mesh.
    static std::queue<ChunkPosition> _update_chunk_mesh_requests;
    static std::queue<std::future<UpdateChunkMeshResults>> _update_chunk_mesh_tasks;

    static std::shared_ptr<zth::gl::Texture2D> _blocks_texture;
    static std::shared_ptr<zth::Material> _chunk_material;

    static zth::Timer _generate_chunks_timer;
    static inline bool _all_chunks_generated = false;

private:
    [[nodiscard]] static auto get_chunk(ChunkPosition position) -> Optional<Reference<Chunk>>;
    [[nodiscard]] static auto get_chunk_unchecked(ChunkPosition position) noexcept -> Chunk&;
    [[nodiscard]] static auto get_neighbors(ChunkPosition position) -> NeighboringChunksData;

    [[nodiscard]] static auto get_chunk_entity(ChunkPosition position) -> Optional<zth::EntityHandle>;
    [[nodiscard]] static auto get_chunk_entity_unchecked(ChunkPosition position) noexcept -> zth::EntityHandle&;

    static auto request_to_generate_new_chunk(ChunkPosition position) -> void;
    static auto request_to_update_chunk_mesh(ChunkPosition position) -> void;
    static auto request_to_update_neighboring_chunks_meshes(ChunkPosition position) -> void;

    [[nodiscard]] static auto generate_new_chunk(ChunkPosition position) -> GenerateChunkResults;
    [[nodiscard]] static auto update_chunk_mesh(ChunkPosition position, const UpdateChunkMeshInputs& inputs)
        -> UpdateChunkMeshResults;

    static auto create_chunk_entity(zth::Scene& scene, const Chunk& chunk, ChunkPosition position) -> void;
    static auto update_chunk_entity_mesh(const Chunk& chunk, ChunkPosition position) -> void;
};
