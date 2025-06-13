/**
 * @file scripts/world_manager.hpp
 * @brief Declares the WorldManager script class for managing world chunks, loading, unloading, and updating.
 */

#pragma once

#include <future>
#include <thread>

#include "hash.hpp"
#include "world/chunk.hpp"

namespace scripts {

/**
 * @brief Chunk management process and design overview.
 *
 * Chunk component consists of a shared pointer to the chunk's data and an array of shared pointers to the neighboring
 * chunks. When a new chunk entity is created, the pointer to the chunk's data is not set, but the pointers to the
 * neighboring chunks are set if these chunks already exist. Whenever world manager finishes loading the chunk's data,
 * it updates the chunk's data pointer and also the neighbor arrays of neighboring chunks. It also pushes onto the
 * update queue the coordinates of the loaded chunk and the neighboring chunks.
 *
 * World manager holds a map which associates a chunk's coordinates with its entity handle. It also keeps separate
 * queues of the coordinates of chunks to unload, load and update (updating a chunk means generating a mesh for it).
 * There's no locking mechanism as all the update operations which run on a separate thread only handle generating a
 * mesh for the chunk, so they only need read access to the data and the data getting updated at the same time as the
 * mesh is being generated isn't an issue since modifying the data means that the chunk is going to be updated again
 * later anyway.
 *
 * World manager performs these steps on every update in order:
 *
 * 1. --- Determine which chunks need to be loaded and which ones need to be unloaded ---
 *     - Iterate over all the chunk coordinates which are within a specified distance from the player and push them onto
 *     the load chunk queue. The chunks closest to the player should be pushed onto the front of the queue.
 *
 * 2. --- Unload chunks ---
 *     - Go through unload chunk requests and remove the entity handles from the map along with destroying these
 *     entities.
 *
 * 3. --- Load chunks ---
 *     - Go through load chunk requests and process them if the number of running load chunk tasks is less than N and if
 *     the requested chunk's position is within the specified distance from the player. Insert a chunk entity entry into
 *     the map. If an entry for that coordinate already exists, skip this request.
 *     - Emplace a chunk component onto the entity without the chunk data, but update the neighbor array to hold
 *     pointers to the data of the chunks which already exist.
 *     - Create and run a load chunk task on a separate thread.
 *
 * 4. --- Get load chunk results ---
 *     - Go through first N load chunk tasks and if the result is ready, update the corresponding chunk's data pointer,
 *     and update the neighbor arrays of neighboring chunks to hold a reference to the data of the chunk that was just
 *     loaded. Add the chunk and neighboring chunks to the update queue.
 *
 * 5. --- Update chunk ---
 *     - Go through update chunk requests and process them if the number of running update chunk tasks is less than N.
 *     If an entity with the provided coordinates is not found in the map, skip this request.
 *     - Create and run an update chunk task on a separate thread.
 *
 * 6. --- Get update chunk results ---
 *     - Go through first N update chunk tasks and if the result is ready, update the corresponding chunk's mesh
 *     renderer component with the generated mesh (This always has to be done on the main thread).
 */

/**
 * @class WorldManager
 * @brief Script class responsible for managing world chunks, including loading, unloading, and updating.
 *
 * Handles chunk entity creation, mesh generation, neighbor management, and chunk queues for efficient world streaming.
 * The chunk management process is described in detail in the comments above the class.
 */
class WorldManager : public zth::Script
{
public:
    /**
     * @brief The player entity handle.
     */
    zth::EntityHandle player;

    /**
     * @brief The starting position for the player in world coordinates.
     */
    glm::vec3 starting_player_position{ 0.0f, 300.0f, 0.0f };

    /**
     * @brief The chunk loading distance from the player.
     */
    i32 distance = 3;

    /**
     * @brief Maximum number of concurrent load chunk tasks.
     */
    usize max_load_chunk_tasks = std::max(std::thread::hardware_concurrency() * 2u, 4u);

    /**
     * @brief Maximum number of concurrent update chunk tasks.
     */
    usize max_update_chunk_tasks = max_load_chunk_tasks;

    /**
     * @brief Maximum number of chunks loaded per frame.
     */
    usize max_chunks_loaded_each_frame = max_load_chunk_tasks;

    /**
     * @brief Maximum number of chunks updated per frame.
     */
    usize max_chunks_updated_each_frame = max_update_chunk_tasks;

    /**
     * @brief Mouse button used to destroy a block.
     */
    zth::MouseButton destroy_block_button = zth::MouseButton::Left;

    /**
     * @brief Mouse button used to place a block.
     */
    zth::MouseButton place_block_button = zth::MouseButton::Right;

public:
    /**
     * @brief Default constructor.
     */
    explicit WorldManager() = default;

    /**
     * @brief Constructs a WorldManager with a player entity handle.
     * @param player The player entity handle.
     */
    explicit WorldManager(zth::EntityHandle player);

    /**
     * @brief Allows editing of world manager parameters in a debug UI.
     */
    auto debug_edit() -> void override;

    /**
     * @brief Handles events for the world manager script.
     * @param actor The entity handle for the world manager.
     * @param event The event to process.
     */
    auto on_event(zth::EntityHandle actor, const zth::Event& event) -> void override;

    /**
     * @brief Called on each update tick for chunk management.
     * @param actor The entity handle for the world manager.
     */
    auto on_update(zth::EntityHandle actor) -> void override;

private:
    /**
     * @brief Pointer to the current scene.
     */
    zth::Scene* _scene = nullptr;

    /**
     * @brief Map from chunk coordinates to entity handles.
     */
    zth::UnorderedMap<glm::ivec2, zth::EntityHandle> _chunk_map;

    /**
     * @brief Queue of chunk positions to unload.
     */
    zth::Deque<glm::ivec2> _unload_chunk_requests;

    /**
     * @brief Queue of chunk positions to load.
     */
    zth::Deque<glm::ivec2> _load_chunk_requests;

    /**
     * @brief Queue of futures for load chunk tasks.
     */
    zth::Deque<std::future<std::pair<glm::ivec2, std::shared_ptr<ChunkData>>>> _load_chunk_tasks;

    /**
     * @brief Queue of chunk positions to update.
     */
    zth::Deque<glm::ivec2> _update_chunk_requests;

    /**
     * @brief Queue of futures for update chunk tasks.
     */
    zth::Deque<std::future<std::pair<zth::EntityHandle, zth::Vector<zth::StandardVertex>>>> _update_chunk_tasks;

    // @todo: Should world manager manage these resources?
    // @todo: Add these to debug menu.

    /**
     * @brief Shared pointer to the blocks texture used for chunk rendering.
     */
    std::shared_ptr<zth::gl::Texture2D> _blocks_texture;

    /**
     * @brief Shared pointer to the chunk material.
     */
    std::shared_ptr<zth::Material> _chunk_material;

private:
    /**
     * @brief Called when the script is attached to an entity.
     * @param actor The entity handle to which the script is attached.
     */
    auto on_attach(zth::EntityHandle actor) -> void override;

    /**
     * @brief Called when the script is detached from an entity.
     * @param actor The entity handle from which the script is detached.
     */
    auto on_detach(zth::EntityHandle actor) -> void override;

    /**
     * @brief Retrieves the chunk entity handle for the given chunk position.
     * @param chunk_position The chunk coordinates.
     * @return Optional entity handle for the chunk.
     */
    [[nodiscard]] auto get_chunk(glm::ivec2 chunk_position) -> Optional<zth::EntityHandle>;

    /**
     * @brief Retrieves the const chunk entity handle for the given chunk position.
     * @param chunk_position The chunk coordinates.
     * @return Optional const entity handle for the chunk.
     */
    [[nodiscard]] auto get_chunk(glm::ivec2 chunk_position) const -> Optional<zth::ConstEntityHandle>;

    /**
     * @brief Requests loading of chunks around the player's current chunk.
     * @param player_chunk The player's current chunk coordinates.
     */
    auto request_to_load_chunks_around_player(glm::ivec2 player_chunk) -> void;

    /**
     * @brief Requests unloading of chunks that are too far from the player.
     * @param player_chunk The player's current chunk coordinates.
     */
    auto request_to_unload_chunks_too_far_away_from_player(glm::ivec2 player_chunk) -> void;

    /**
     * @brief Requests loading of a chunk at the given position.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_load_chunk(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Requests loading of a chunk at the given position with high priority.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_load_chunk_with_priority(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Launches an asynchronous task to load a chunk.
     * @param chunk_position The chunk coordinates.
     */
    auto launch_load_chunk_task(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Loads chunk data for the given chunk position.
     * @param chunk_position The chunk coordinates.
     * @return Pair of chunk coordinates and shared pointer to the loaded ChunkData.
     */
    [[nodiscard]] static auto load_chunk(glm::ivec2 chunk_position)
        -> std::pair<glm::ivec2, std::shared_ptr<ChunkData>>;

    /**
     * @brief Creates a new chunk entity at the given chunk position.
     * @param chunk_position The chunk coordinates.
     * @return The created entity handle.
     */
    [[nodiscard]] auto create_new_chunk_entity(glm::ivec2 chunk_position) -> zth::EntityHandle;

    /**
     * @brief Updates the chunk entity with loaded chunk data.
     * @param chunk_entity The chunk entity handle.
     * @param chunk_data The loaded chunk data.
     */
    static auto update_chunk_entity_with_data(zth::EntityHandle chunk_entity, std::shared_ptr<ChunkData>&& chunk_data)
        -> void;

    /**
     * @brief Updates the neighbor arrays of neighboring chunks when a chunk is loaded.
     * @param chunk_entity The loaded chunk entity handle.
     */
    auto update_neighbor_arrays_on_chunk_loaded(zth::EntityHandle chunk_entity) -> void;

    /**
     * @brief Requests updating of a chunk at the given position.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_update_chunk(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Requests updating of a chunk at the given position with high priority.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_update_chunk_with_priority(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Requests updating of all neighboring chunks.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_update_neighbors(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Requests updating of all neighboring chunks with high priority.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_update_neighbors_with_priority(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Launches an asynchronous task to update a chunk's mesh.
     * @param chunk_entity The chunk entity handle.
     */
    auto launch_update_chunk_task(zth::EntityHandle chunk_entity) -> void;

    /**
     * @brief Generates a mesh for the chunk and returns it with the entity handle.
     * @param chunk_entity The chunk entity handle.
     * @param chunk_data The chunk data.
     * @param neighbors The array of neighboring chunk data.
     * @return Pair of entity handle and generated mesh.
     */
    [[nodiscard]] static auto update_chunk(zth::EntityHandle chunk_entity, const ChunkData& chunk_data,
                                           const NeighborsArray& neighbors)
        -> std::pair<zth::EntityHandle, zth::Vector<zth::StandardVertex>>;

    /**
     * @brief Updates the chunk entity's mesh renderer and collider with the new mesh.
     * @param chunk_entity The chunk entity handle.
     * @param chunk_mesh The generated mesh.
     */
    static auto update_chunk_entity(zth::EntityHandle chunk_entity, const zth::Vector<zth::StandardVertex>& chunk_mesh)
        -> void;

    /**
     * @brief Requests unloading of a chunk at the given position.
     * @param chunk_position The chunk coordinates.
     */
    auto request_to_unload_chunk(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Unloads a chunk at the given position, destroying its entity and removing it from the map.
     * @param chunk_position The chunk coordinates.
     */
    auto unload_chunk(glm::ivec2 chunk_position) -> void;

    /**
     * @brief Returns the coordinates of the chunk that the player is currently in.
     * @return The player's current chunk coordinates.
     */
    [[nodiscard]] auto get_player_chunk() const -> glm::ivec2;

    /**
     * @brief Computes the distance between two chunks (in chunk coordinates).
     * @param chunk_a The first chunk coordinates.
     * @param chunk_b The second chunk coordinates.
     * @return The maximum of the absolute differences in x and z.
     */
    [[nodiscard]] static auto get_distance(glm::ivec2 chunk_a, glm::ivec2 chunk_b) -> i32;

    /**
     * @brief Retrieves the array of neighboring chunk data for a given chunk position.
     * @param chunk_position The chunk coordinates.
     * @return Array of shared pointers to neighboring chunk data.
     */
    [[nodiscard]] auto get_neighbors(glm::ivec2 chunk_position) const -> NeighborsArray;

    /**
     * @brief Clears all loaded chunks and resets the world manager state.
     */
    auto clear_world() -> void;

    /**
     * @brief Handles mouse button pressed events for block placement and destruction.
     * @param actor The entity handle for the world manager.
     * @param event The mouse button pressed event.
     */
    auto on_mouse_button_pressed_event(zth::EntityHandle actor, const zth::MouseButtonPressedEvent& event) -> void;
};

} // namespace scripts
