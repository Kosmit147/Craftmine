#pragma once

#include "fwd.hpp"

enum class BlockType : u8
{
    Air = 0,
    Grass,
    Dirt,
    Stone,
};

// This is a bitmask type.
enum BlockFacing : u8
{
    Facing_None = 0,

    // Side faces.
    Facing_Backward = 1 << 0,
    Facing_Forward = 1 << 1,
    Facing_Left = 1 << 2,
    Facing_Right = 1 << 3,

    // Up and down faces.
    Facing_Down = 1 << 4,
    Facing_Up = 1 << 5,
};

// clang-format off
inline auto operator|=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs | rhs); }
inline auto operator&=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs & rhs); }
// clang-format on

[[nodiscard]] auto side_facing(BlockFacing facing) -> bool;

constexpr inline glm::ivec3 chunk_size{ 16, 256, 16 };
constexpr inline i32 blocks_in_chunk = chunk_size.x * chunk_size.y * chunk_size.z;

struct ChunkPosition
{
    i32 x;
    i32 z;

    [[nodiscard]] auto operator==(const ChunkPosition&) const -> bool = default;
};

template<> struct std::hash<ChunkPosition>
{
    [[nodiscard]] auto operator()(const ChunkPosition& pos) const noexcept -> std::size_t
    {
        auto h1 = std::hash<i32>{}(pos.x);
        auto h2 = std::hash<i32>{}(pos.z);

        // Similar to boost::hash_combine.
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

struct ChunkMesh
{
    zth::Vector<zth::StandardVertex> vertices;
};

struct NeighboringChunksData
{
    std::shared_ptr<const ChunkData> backward_chunk_data; // +z
    std::shared_ptr<const ChunkData> forward_chunk_data;  // -z
    std::shared_ptr<const ChunkData> left_chunk_data;     // -x
    std::shared_ptr<const ChunkData> right_chunk_data;    // +x
};

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
    [[nodiscard]] auto at_exterior(glm::ivec3 coordinates, const NeighboringChunksData& neighbors) const
        -> Optional<Reference<const BlockType>>;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) -> BlockType&;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) const -> const BlockType&;

    [[nodiscard]] auto generate_mesh(const NeighboringChunksData& neighbors) const -> ChunkMesh;

    [[nodiscard]] static auto valid_coordinates(glm::ivec3 coordinates) -> bool;

private:
    BlocksArray _data; // Purposefully left uninitialized.

private:
    using WallCoordinates =
        std::ranges::cartesian_product_view<std::ranges::iota_view<int, int>, std::ranges::iota_view<int, int>,
                                            std::ranges::iota_view<int, int>>;

    [[nodiscard]] auto visible_faces_for_exterior_block(glm::ivec3 coordinates,
                                                        const NeighboringChunksData& neighbors) const -> BlockFacing;
    [[nodiscard]] auto visible_faces_for_interior_block(glm::ivec3 coordinates) const -> BlockFacing;

    // Mesh generation.
    auto append_vertices_for_exterior_blocks(ChunkMesh& mesh, const NeighboringChunksData& neighbors) const -> void;
    auto append_vertices_for_interior_blocks(ChunkMesh& mesh) const -> void;

    auto append_vertices_for_wall(ChunkMesh& mesh, const WallCoordinates& wall,
                                  const NeighboringChunksData& neighbors) const -> void;

    [[nodiscard]] static auto exterior_block_coordinates(glm::ivec3 coordinates) -> bool;
    [[nodiscard]] static auto exterior_x(i32 x) -> bool;
    [[nodiscard]] static auto exterior_y(i32 y) -> bool;
    [[nodiscard]] static auto exterior_z(i32 z) -> bool;

    [[nodiscard]] static auto interior_block_coordinates(glm::ivec3 coordinates) -> bool;
    [[nodiscard]] static auto interior_x(i32 x) -> bool;
    [[nodiscard]] static auto interior_y(i32 y) -> bool;
    [[nodiscard]] static auto interior_z(i32 z) -> bool;
};

class Chunk
{
public:
    explicit Chunk(std::shared_ptr<ChunkData>&& data);

    ZTH_NO_COPY(Chunk)
    ZTH_DEFAULT_MOVE(Chunk)

    ~Chunk() = default;

    // This must be done on the main thread.
    auto upload_mesh(const ChunkMesh& mesh) -> void;

    [[nodiscard]] static auto to_world_x(i32 x) -> i32;
    [[nodiscard]] static auto to_world_z(i32 z) -> i32;

    [[nodiscard]] auto data(this auto&& self) -> auto& { return self._data; }
    [[nodiscard]] auto mesh() const -> const auto& { return _mesh; }

private:
    std::shared_ptr<ChunkData> _data;

    // @todo: Fix this hack.
    std::shared_ptr<zth::QuadMesh> _mesh =
        std::make_shared<zth::QuadMesh>(std::array{ zth::StandardVertex{} }, zth::StandardVertex::layout);
};

class ChunkGenerator
{
public:
    static inline float scale = 0.015f;
    static inline float min_height = 0.3f;
    static inline float max_height = 0.45f;

public:
    ChunkGenerator() = delete;

    [[nodiscard]] static auto generate(ChunkPosition position) -> std::shared_ptr<ChunkData>;

private:
    [[nodiscard]] static auto get_height(i32 world_x, i32 world_z) -> i32;
};
