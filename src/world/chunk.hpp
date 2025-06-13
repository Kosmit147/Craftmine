/**
 * @file chunk.hpp
 * @brief Defines chunk data structures and functions for world representation and manipulation.
 */

#pragma once

#include "fwd.hpp"
#include "world/block.hpp"

/**
 * @brief The size of a chunk in blocks (width, height, depth).
 */
constexpr inline glm::ivec3 chunk_size{ 16, 256, 16 };

/**
 * @brief The total number of blocks in a chunk.
 */
constexpr inline i32 blocks_in_chunk = chunk_size.x * chunk_size.y * chunk_size.z;

// These are used to access the chunk component's neighbors array.
/**
 * @brief Index for the neighbor in the +X direction.
 */
constexpr inline usize plus_x_idx = 0;
/**
 * @brief Index for the neighbor in the -X direction.
 */
constexpr inline usize minus_x_idx = 1;
/**
 * @brief Index for the neighbor in the +Z direction.
 */
constexpr inline usize plus_z_idx = 2;
/**
 * @brief Index for the neighbor in the -Z direction.
 */
constexpr inline usize minus_z_idx = 3;

/**
 * @brief The number of neighbors for a chunk (4 in the XZ plane).
 */
constexpr inline usize neighbor_count = 4;

/**
 * @brief Offsets for each neighbor in the XZ plane.
 */
constexpr inline std::array neighbor_offsets = {
    glm::ivec2{ 1, 0 },  // Plus X.
    glm::ivec2{ -1, 0 }, // Minus X.
    glm::ivec2{ 0, 1 },  // Plus Z.
    glm::ivec2{ 0, -1 }, // Minus Z.
};

/**
 * @brief Indices of opposite neighbors for each direction.
 */
constexpr inline std::array opposite_neighbor_offset_idx = {
    1,
    0,
    3,
    2,
};

/**
 * @brief Array type for holding pointers to neighboring chunk data.
 */
using NeighborsArray = std::array<std::shared_ptr<const ChunkData>, neighbor_count>;

/**
 * @class ChunkData
 * @brief Stores and manages the block data for a single chunk.
 */
class ChunkData
{
public:
    /**
     * @brief Array type for storing all blocks in the chunk.
     */
    using BlocksArray = std::array<BlockType, blocks_in_chunk>;

    /**
     * @brief Default constructor.
     */
    explicit ChunkData() = default;

    /**
     * @brief Deleted copy constructor and copy assignment.
     */
    ZTH_NO_COPY(ChunkData)
    /**
     * @brief Defaulted move constructor and move assignment.
     */
    ZTH_DEFAULT_MOVE(ChunkData)

    /**
     * @brief Default destructor.
     */
    ~ChunkData() = default;

    /**
     * @brief Returns a reference to the block at the given coordinates, if valid.
     * @param coordinates The block coordinates within the chunk.
     * @return Optional reference to the block.
     */
    [[nodiscard]] auto at(glm::ivec3 coordinates) -> Optional<Reference<BlockType>>;

    /**
     * @brief Returns a const reference to the block at the given coordinates, if valid.
     * @param coordinates The block coordinates within the chunk.
     * @return Optional const reference to the block.
     */
    [[nodiscard]] auto at(glm::ivec3 coordinates) const -> Optional<Reference<const BlockType>>;

    /**
     * @brief Returns a const reference to a block at the given coordinates, possibly from a neighbor.
     * @param coordinates The block coordinates (may be outside this chunk).
     * @param neighbors Array of neighboring chunk data.
     * @return Optional const reference to the block.
     */
    [[nodiscard]] auto at_exterior(glm::ivec3 coordinates, const NeighborsArray& neighbors) const
        -> Optional<Reference<const BlockType>>;

    /**
     * @brief Returns a mutable reference to the block at the given coordinates.
     * @param coordinates The block coordinates within the chunk.
     * @return Reference to the block.
     */
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) -> BlockType&;

    /**
     * @brief Returns a const reference to the block at the given coordinates.
     * @param coordinates The block coordinates within the chunk.
     * @return Const reference to the block.
     */
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) const -> const BlockType&;

    /**
     * @brief Generates a mesh for the chunk based on visible faces.
     * @param neighbors Array of neighboring chunk data.
     * @return Vector of standard vertices representing the mesh.
     */
    [[nodiscard]] auto generate_mesh(const NeighborsArray& neighbors) const -> zth::Vector<zth::StandardVertex>;

    /**
     * @brief Checks if the given coordinates are valid within the chunk.
     * @param coordinates The block coordinates.
     * @return True if valid, false otherwise.
     */
    [[nodiscard]] static auto valid_coordinates(glm::ivec3 coordinates) -> bool;

private:
    /**
     * @brief The array of block data for this chunk.
     * @details Purposefully left uninitialized for performance.
     */
    BlocksArray _data;

private:
    /**
     * @brief Type alias for wall coordinates used in mesh generation.
     */
    using WallCoordinates =
        std::ranges::cartesian_product_view<std::ranges::iota_view<int, int>, std::ranges::iota_view<int, int>,
                                            std::ranges::iota_view<int, int>>;

    /**
     * @brief Determines visible faces for an exterior block, considering neighbors.
     * @param coordinates The block coordinates.
     * @param neighbors Array of neighboring chunk data.
     * @return Bitmask of visible block facings.
     */
    [[nodiscard]] auto visible_faces_for_exterior_block(glm::ivec3 coordinates, const NeighborsArray& neighbors) const
        -> BlockFacing;

    /**
     * @brief Determines visible faces for an interior block.
     * @param coordinates The block coordinates.
     * @return Bitmask of visible block facings.
     */
    [[nodiscard]] auto visible_faces_for_interior_block(glm::ivec3 coordinates) const -> BlockFacing;

    // Mesh generation.

    /**
     * @brief Appends vertices for all exterior blocks to the mesh.
     * @param vertices The vertex vector to append to.
     * @param neighbors Array of neighboring chunk data.
     */
    auto append_vertices_for_exterior_blocks(zth::Vector<zth::StandardVertex>& vertices,
                                             const NeighborsArray& neighbors) const -> void;

    /**
     * @brief Appends vertices for all interior blocks to the mesh.
     * @param vertices The vertex vector to append to.
     */
    auto append_vertices_for_interior_blocks(zth::Vector<zth::StandardVertex>& vertices) const -> void;

    /**
     * @brief Appends vertices for a wall of blocks to the mesh.
     * @param vertices The vertex vector to append to.
     * @param wall The wall coordinates.
     * @param neighbors Array of neighboring chunk data.
     */
    auto append_vertices_for_wall(zth::Vector<zth::StandardVertex>& vertices, const WallCoordinates& wall,
                                  const NeighborsArray& neighbors) const -> void;

    /**
     * @brief Checks if the given coordinates are on the exterior of the chunk.
     * @param coordinates The block coordinates.
     * @return True if exterior, false otherwise.
     */
    [[nodiscard]] static auto exterior_block_coordinates(glm::ivec3 coordinates) -> bool;

    /**
     * @brief Checks if the given x coordinate is on the exterior.
     * @param x The x coordinate.
     * @return True if exterior, false otherwise.
     */
    [[nodiscard]] static auto exterior_x(i32 x) -> bool;

    /**
     * @brief Checks if the given y coordinate is on the exterior.
     * @param y The y coordinate.
     * @return True if exterior, false otherwise.
     */
    [[nodiscard]] static auto exterior_y(i32 y) -> bool;

    /**
     * @brief Checks if the given z coordinate is on the exterior.
     * @param z The z coordinate.
     * @return True if exterior, false otherwise.
     */
    [[nodiscard]] static auto exterior_z(i32 z) -> bool;

    /**
     * @brief Checks if the given coordinates are in the interior of the chunk.
     * @param coordinates The block coordinates.
     * @return True if interior, false otherwise.
     */
    [[nodiscard]] static auto interior_block_coordinates(glm::ivec3 coordinates) -> bool;

    /**
     * @brief Checks if the given x coordinate is in the interior.
     * @param x The x coordinate.
     * @return True if interior, false otherwise.
     */
    [[nodiscard]] static auto interior_x(i32 x) -> bool;

    /**
     * @brief Checks if the given y coordinate is in the interior.
     * @param y The y coordinate.
     * @return True if interior, false otherwise.
     */
    [[nodiscard]] static auto interior_y(i32 y) -> bool;

    /**
     * @brief Checks if the given z coordinate is in the interior.
     * @param z The z coordinate.
     * @return True if interior, false otherwise.
     */
    [[nodiscard]] static auto interior_z(i32 z) -> bool;
};

/**
 * @brief Converts a world x coordinate to a chunk x coordinate.
 * @param x The world x coordinate.
 * @return The chunk x coordinate.
 */
[[nodiscard]] auto world_x_to_chunk_x(i32 x) -> i32;

/**
 * @brief Converts a world z coordinate to a chunk z coordinate.
 * @param z The world z coordinate.
 * @return The chunk z coordinate.
 */
[[nodiscard]] auto world_z_to_chunk_z(i32 z) -> i32;

/**
 * @brief Converts a chunk x coordinate to a world x coordinate.
 * @param x The chunk x coordinate.
 * @return The world x coordinate.
 */
[[nodiscard]] auto chunk_x_to_world_x(i32 x) -> i32;

/**
 * @brief Converts a chunk z coordinate to a world z coordinate.
 * @param z The chunk z coordinate.
 * @return The world z coordinate.
 */
[[nodiscard]] auto chunk_z_to_world_z(i32 z) -> i32;

/**
 * @struct ChunkComponent
 * @brief ECS component for associating chunk data and neighbors with an entity.
 */
struct ChunkComponent
{
    /**
     * @brief Shared pointer to the chunk's block data.
     */
    std::shared_ptr<ChunkData> data = nullptr;
    /**
     * @brief Array of pointers to neighboring chunk data.
     */
    NeighborsArray neighbors{};
    /**
     * @brief The chunk's position in chunk coordinates (XZ plane).
     */
    glm::ivec2 position{ 0, 0 };

    /**
     * @brief Places a block at the given world position.
     * @param world_position The world position to place the block.
     */
    auto place_block(glm::vec3 world_position) -> void;

    /**
     * @brief Destroys a block at the given world position.
     * @param world_position The world position to destroy the block.
     */
    auto destroy_block(glm::vec3 world_position) -> void;
};
