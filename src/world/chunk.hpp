#pragma once

#include "fwd.hpp"

#include "world/block.hpp"

constexpr inline glm::ivec3 chunk_size{ 16, 256, 16 };
constexpr inline i32 blocks_in_chunk = chunk_size.x * chunk_size.y * chunk_size.z;

// These are used to access the chunk component's neighbors array.
constexpr inline usize plus_x_idx = 0;
constexpr inline usize minus_x_idx = 1;
constexpr inline usize plus_z_idx = 2;
constexpr inline usize minus_z_idx = 3;

constexpr inline usize neighbor_count = 4;

constexpr inline std::array neighbor_offsets = {
    glm::ivec2{ 1, 0 },  // Plus X.
    glm::ivec2{ -1, 0 }, // Minus X.
    glm::ivec2{ 0, 1 },  // Plus Z.
    glm::ivec2{ 0, -1 }, // Minus Z.
};

constexpr inline std::array opposite_neighbor_offset_idx = {
    1,
    0,
    3,
    2,
};

using NeighborsArray = std::array<std::shared_ptr<const ChunkData>, neighbor_count>;

class ChunkData
{
public:
    using BlocksArray = std::array<BlockType, blocks_in_chunk>;

    explicit ChunkData() = default;

    ZTH_NO_COPY(ChunkData)
    ZTH_DEFAULT_MOVE(ChunkData)

    ~ChunkData() = default;

    [[nodiscard]] auto at(glm::ivec3 coordinates) -> Optional<Reference<BlockType>>;
    [[nodiscard]] auto at(glm::ivec3 coordinates) const -> Optional<Reference<const BlockType>>;
    [[nodiscard]] auto at_exterior(glm::ivec3 coordinates, const NeighborsArray& neighbors) const
        -> Optional<Reference<const BlockType>>;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) -> BlockType&;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) const -> const BlockType&;

    [[nodiscard]] auto generate_mesh(const NeighborsArray& neighbors) const -> zth::Vector<zth::StandardVertex>;

    [[nodiscard]] static auto valid_coordinates(glm::ivec3 coordinates) -> bool;

private:
    BlocksArray _data; // Purposefully left uninitialized.

private:
    using WallCoordinates =
        std::ranges::cartesian_product_view<std::ranges::iota_view<int, int>, std::ranges::iota_view<int, int>,
                                            std::ranges::iota_view<int, int>>;

    [[nodiscard]] auto visible_faces_for_exterior_block(glm::ivec3 coordinates, const NeighborsArray& neighbors) const
        -> BlockFacing;
    [[nodiscard]] auto visible_faces_for_interior_block(glm::ivec3 coordinates) const -> BlockFacing;

    // Mesh generation.
    auto append_vertices_for_exterior_blocks(zth::Vector<zth::StandardVertex>& vertices,
                                             const NeighborsArray& neighbors) const -> void;
    auto append_vertices_for_interior_blocks(zth::Vector<zth::StandardVertex>& vertices) const -> void;

    auto append_vertices_for_wall(zth::Vector<zth::StandardVertex>& vertices, const WallCoordinates& wall,
                                  const NeighborsArray& neighbors) const -> void;

    [[nodiscard]] static auto exterior_block_coordinates(glm::ivec3 coordinates) -> bool;
    [[nodiscard]] static auto exterior_x(i32 x) -> bool;
    [[nodiscard]] static auto exterior_y(i32 y) -> bool;
    [[nodiscard]] static auto exterior_z(i32 z) -> bool;

    [[nodiscard]] static auto interior_block_coordinates(glm::ivec3 coordinates) -> bool;
    [[nodiscard]] static auto interior_x(i32 x) -> bool;
    [[nodiscard]] static auto interior_y(i32 y) -> bool;
    [[nodiscard]] static auto interior_z(i32 z) -> bool;
};

[[nodiscard]] auto world_x_to_chunk_x(i32 x) -> i32;
[[nodiscard]] auto world_z_to_chunk_z(i32 z) -> i32;

[[nodiscard]] auto chunk_x_to_world_x(i32 x) -> i32;
[[nodiscard]] auto chunk_z_to_world_z(i32 z) -> i32;

struct ChunkComponent
{
    std::shared_ptr<ChunkData> data = nullptr;
    NeighborsArray neighbors{};
    glm::ivec2 position{ 0, 0 };
};
