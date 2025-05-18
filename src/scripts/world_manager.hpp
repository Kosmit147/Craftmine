#pragma once

#include <future>
#include <thread>

#include "hash.hpp"
#include "world/chunk.hpp"

namespace scripts {

// Chunk management process.
//
// Chunk component consists of a shared pointer to the chunk's data and an array of shared pointers to the neighboring
// chunks. When a new chunk entity is created, the pointer to the chunk's data is not set, but the pointers to the
// neighboring chunks are set if these chunks already exist. Whenever world manager finishes loading the chunk's data,
// it updates the chunk's data pointer and also the neighbor arrays of neighboring chunks. It also pushes onto the
// update queue the coordinates of the loaded chunk and the neighboring chunks.
//
// World manager holds a map which associates a chunk's coordinates with its entity handle. It also keeps separate
// queues of the coordinates of chunks to unload, load and update (updating a chunk means generating a mesh for it).
// There's no locking mechanism as all the update operations which run on a separate thread only handle generating a
// mesh for the chunk, so they only need read access to the data and the data getting updated at the same time as the
// mesh is being generated isn't an issue since modifying the data means that the chunk is going to be updated again
// later anyway.
//
// World manager performs these steps on every update in order:
//
// 1. --- Determine which chunks need to be loaded and which ones need to be unloaded ---
//     - Iterate over all the chunk coordinates which are within a specified distance from the player and push them onto
//     the load chunk queue. The chunks closest to the player should be pushed onto the front of the queue.
//
// 2. --- Unload chunks ---
//     - Go through unload chunk requests and remove the entity handles from the map along with destroying these
//     entities.
//
// 3. --- Load chunks ---
//     - Go through load chunk requests and process them if the number of running load chunk tasks is less than N and if
//     the requested chunk's position is within the specified distance from the player. Insert a chunk entity entry into
//     the map. If an entry for that coordinate already exists, skip this request.
//     - Emplace a chunk component onto the entity without the chunk data, but update the neighbor array to hold
//     pointers to the data of the chunks which already exist.
//     - Create and run a load chunk task on a separate thread.
//
// 4. --- Get load chunk results ---
//     - Go through first N load chunk tasks and if the result is ready, update the corresponding chunk's data pointer,
//     and update the neighbor arrays of neighboring chunks to hold a reference to the data of the chunk that was just
//     loaded. Add the chunk and neighboring chunks to the update queue.
//
// 5. --- Update chunk ---
//     - Go through update chunk requests and process them if the number of running update chunk tasks is less than N.
//     If an entity with the provided coordinates is not found in the map, skip this request.
//     - Create and run an update chunk task on a separate thread.
//
// 6. --- Get update chunk results ---
//     - Go through first N update chunk tasks and if the result is ready, update the corresponding chunk's mesh
//     renderer component with the generated mesh (This always has to be done on the main thread).

class WorldManager : public zth::Script
{
public:
    // @todo: Add player to debug menu.
    zth::ConstEntityHandle player;
    i32 distance = 4;

    usize max_load_chunk_tasks = std::max(std::thread::hardware_concurrency() * 2u, 4u);
    usize max_update_chunk_tasks = max_load_chunk_tasks;

    usize max_chunks_loaded_each_frame = max_load_chunk_tasks;
    usize max_chunks_updated_each_frame = max_update_chunk_tasks;

public:
    explicit WorldManager() = default;
    explicit WorldManager(zth::ConstEntityHandle player);

    auto debug_edit() -> void override;

    auto on_update(zth::EntityHandle actor) -> void override;

private:
    zth::Scene* _scene = nullptr;
    zth::UnorderedMap<glm::ivec2, zth::EntityHandle> _chunk_map;

    zth::Deque<glm::ivec2> _unload_chunk_requests;

    zth::Deque<glm::ivec2> _load_chunk_requests;
    zth::Deque<std::future<std::pair<glm::ivec2, std::shared_ptr<ChunkData>>>> _load_chunk_tasks;

    zth::Deque<glm::ivec2> _update_chunk_requests;
    zth::Deque<std::future<std::pair<zth::EntityHandle, zth::Vector<zth::StandardVertex>>>> _update_chunk_tasks;

    // @todo: Should world manager manage these resources?
    // @todo: Add these to debug menu.
    std::shared_ptr<zth::gl::Texture2D> _blocks_texture;
    std::shared_ptr<zth::Material> _chunk_material;

private:
    auto on_attach(zth::EntityHandle actor) -> void override;
    auto on_detach(zth::EntityHandle actor) -> void override;

    [[nodiscard]] auto get_chunk(glm::ivec2 chunk_position) -> Optional<zth::EntityHandle>;
    [[nodiscard]] auto get_chunk(glm::ivec2 chunk_position) const -> Optional<zth::ConstEntityHandle>;

    auto request_to_load_chunks_around_player(glm::ivec2 player_chunk) -> void;
    auto request_to_unload_chunks_too_far_away_from_player(glm::ivec2 player_chunk) -> void;

    auto request_to_load_chunk(glm::ivec2 chunk_position) -> void;
    auto request_to_load_chunk_with_priority(glm::ivec2 chunk_position) -> void;
    auto launch_load_chunk_task(glm::ivec2 chunk_position) -> void;
    [[nodiscard]] static auto load_chunk(glm::ivec2 chunk_position)
        -> std::pair<glm::ivec2, std::shared_ptr<ChunkData>>;
    [[nodiscard]] auto create_new_chunk_entity(glm::ivec2 chunk_position) -> zth::EntityHandle;
    static auto update_chunk_entity_with_data(zth::EntityHandle chunk_entity, std::shared_ptr<ChunkData>&& chunk_data)
        -> void;
    auto update_neighbor_arrays_on_chunk_loaded(zth::EntityHandle chunk_entity) -> void;

    auto request_to_update_chunk(glm::ivec2 chunk_position) -> void;
    auto request_to_update_chunk_with_priority(glm::ivec2 chunk_position) -> void;
    auto request_to_update_neighbors(glm::ivec2 chunk_position) -> void;
    auto request_to_update_neighbors_with_priority(glm::ivec2 chunk_position) -> void;
    auto launch_update_chunk_task(zth::EntityHandle chunk_entity) -> void;
    [[nodiscard]] static auto update_chunk(zth::EntityHandle chunk_entity, const ChunkData& chunk_data,
                                           const NeighborsArray& neighbors)
        -> std::pair<zth::EntityHandle, zth::Vector<zth::StandardVertex>>;
    static auto update_chunk_entity(zth::EntityHandle chunk_entity, const zth::Vector<zth::StandardVertex>& chunk_mesh)
        -> void;

    auto request_to_unload_chunk(glm::ivec2 chunk_position) -> void;
    auto unload_chunk(glm::ivec2 chunk_position) -> void;

    // Returns the coordinate of the chunk that the player is in.
    [[nodiscard]] auto get_player_chunk() const -> glm::ivec2;
    [[nodiscard]] static auto get_distance(glm::ivec2 chunk_a, glm::ivec2 chunk_b) -> i32;
    [[nodiscard]] auto get_neighbors(glm::ivec2 chunk_position) const -> NeighborsArray;

    auto clear_world() -> void;
};

} // namespace scripts
