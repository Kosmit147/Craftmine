#pragma once

enum class Block : u8
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

constexpr glm::ivec3 chunk_size{ 16, 256, 16 };
constexpr i32 blocks_in_chunk = chunk_size.x * chunk_size.y * chunk_size.z;

struct ChunkPosition
{
    i32 x;
    i32 z;

    [[nodiscard]] auto operator==(const ChunkPosition&) const -> bool = default;
};

template<> struct std::hash<ChunkPosition>
{
    auto operator()(const ChunkPosition& pos) const noexcept -> std::size_t
    {
        auto h1 = std::hash<i32>{}(pos.x);
        auto h2 = std::hash<i32>{}(pos.z);

        // Similar to boost::hash_combine.
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

using ChunkData = std::array<Block, blocks_in_chunk>;

struct ChunkMesh
{
    zth::Vector<zth::StandardVertex> vertices;
    usize faces;
};

class Chunk
{
public:
    explicit Chunk(zth::UniquePtr<ChunkData>&& blocks);

    ZTH_NO_COPY(Chunk)
    ZTH_DEFAULT_MOVE(Chunk)

    [[nodiscard]] auto generate_mesh() const -> ChunkMesh;
    auto upload_mesh(const ChunkMesh& mesh) -> void;

    [[nodiscard]] auto at(glm::ivec3 coordinates) const -> zth::Optional<Block>;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) -> Block&;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) const -> const Block&;

    [[nodiscard]] static auto to_world_x(i32 x) -> i32;
    [[nodiscard]] static auto to_world_z(i32 z) -> i32;

    [[nodiscard]] auto mesh() const -> const auto& { return _mesh; }

private:
    zth::UniquePtr<ChunkData> _blocks;
    std::shared_ptr<zth::QuadMesh> _mesh;

private:
    [[nodiscard]] auto visible_faces_for_block(glm::ivec3 coordinates) const -> BlockFacing;

    [[nodiscard]] static auto valid_coordinates(glm::ivec3 coordinates) -> bool;
};

class ChunkGenerator
{
public:
    static inline float scale = 0.015f;
    static inline float min_height = 0.3f;
    static inline float max_height = 0.45f;

public:
    ChunkGenerator() = delete;

    [[nodiscard]] static auto generate(ChunkPosition position) -> zth::UniquePtr<ChunkData>;

private:
    [[nodiscard]] static auto get_height(i32 world_x, i32 world_z) -> i32;
};
