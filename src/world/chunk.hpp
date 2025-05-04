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

[[nodiscard]] auto side_facing(BlockFacing facing) -> bool;

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
    [[nodiscard]] auto operator()(const ChunkPosition& pos) const noexcept -> std::size_t
    {
        auto h1 = std::hash<i32>{}(pos.x);
        auto h2 = std::hash<i32>{}(pos.z);

        // Similar to boost::hash_combine.
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

// ChunkData acts as a non-copyable reference type (it's only a pointer to the actual data).
class ChunkData
{
public:
    using BlocksArray = std::array<Block, blocks_in_chunk>;

    explicit ChunkData() = default;
    explicit ChunkData(zth::UniquePtr<BlocksArray>&& data);

    ZTH_NO_COPY(ChunkData)
    ZTH_DEFAULT_MOVE(ChunkData)

    ~ChunkData() = default;

    [[nodiscard]] auto at(glm::ivec3 coordinates) -> Optional<Reference<Block>>;
    [[nodiscard]] auto at(glm::ivec3 coordinates) const -> Optional<Reference<const Block>>;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) -> Block&;
    [[nodiscard]] auto operator[](glm::ivec3 coordinates) const -> const Block&;

    [[nodiscard]] auto generate_mesh() const -> zth::Vector<zth::StandardVertex>;

    [[nodiscard]] static auto valid_coordinates(glm::ivec3 coordinates) -> bool;

private:
    zth::UniquePtr<BlocksArray> _data = zth::make_unique_for_overwrite<BlocksArray>();

private:
    [[nodiscard]] auto visible_faces_for_block(glm::ivec3 coordinates) const -> BlockFacing;
};

class Chunk
{
public:
    explicit Chunk(ChunkData&& data);

    ZTH_NO_COPY(Chunk)
    ZTH_DEFAULT_MOVE(Chunk)

    ~Chunk() = default;

    // This must be done on the main thread.
    auto upload_mesh(const zth::Vector<zth::StandardVertex>& mesh) -> void;

    [[nodiscard]] static auto to_world_x(i32 x) -> i32;
    [[nodiscard]] static auto to_world_z(i32 z) -> i32;

    [[nodiscard]] auto mesh() const -> const auto& { return _mesh; }

private:
    ChunkData _data;
    std::shared_ptr<zth::QuadMesh> _mesh;
};

class ChunkGenerator
{
public:
    static inline float scale = 0.015f;
    static inline float min_height = 0.3f;
    static inline float max_height = 0.45f;

public:
    ChunkGenerator() = delete;

    [[nodiscard]] static auto generate(ChunkPosition position) -> ChunkData;

private:
    [[nodiscard]] static auto get_height(i32 world_x, i32 world_z) -> i32;
};
