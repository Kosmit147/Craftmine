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
    Facing_Backward = 1 << 0,
    Facing_Forward = 1 << 1,
    Facing_Left = 1 << 2,
    Facing_Right = 1 << 3,
    Facing_Down = 1 << 4,
    Facing_Up = 1 << 5,
};

inline auto operator|=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs | rhs); }
inline auto operator&=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs & rhs); }

constexpr glm::ivec3 chunk_size{ 16, 256, 16 };
constexpr i32 blocks_in_chunk = chunk_size.x * chunk_size.y * chunk_size.z;

using ChunkData = std::array<Block, blocks_in_chunk>;

struct ChunkMesh
{
    zth::Vector<zth::StandardVertex> vertices;
    usize faces;
};

class Chunk
{
public:
    explicit Chunk(std::unique_ptr<ChunkData>&& blocks);

    auto generate_mesh() const -> ChunkMesh;
    auto upload_mesh(const ChunkMesh& mesh) -> void;

    auto at(glm::ivec3 coordinates) const -> zth::Optional<Block>;
    auto operator[](glm::ivec3 coordinates) -> Block&;
    auto operator[](glm::ivec3 coordinates) const -> const Block&;

    static auto to_world_x(i32 x) -> i32;
    static auto to_world_z(i32 z) -> i32;

    auto mesh() const -> const auto& { return _mesh; }

private:
    std::unique_ptr<ChunkData> _blocks;
    zth::Mesh _mesh;

private:
    auto visible_faces_for_block(glm::ivec3 coordinates) const -> BlockFacing;

    static auto valid_coordinates(glm::ivec3 coordinates) -> bool;
};

class ChunkGenerator
{
public:
    static inline float scale = 0.015f;
    static inline float min_height = 0.3f;
    static inline float max_height = 0.45f;

public:
    ChunkGenerator() = delete;

    static auto generate(i32 chunk_x, i32 chunk_z) -> std::unique_ptr<ChunkData>;

private:
    static auto get_height(i32 world_x, i32 world_z) -> i32;
};
