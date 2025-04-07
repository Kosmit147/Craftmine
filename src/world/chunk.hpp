#pragma once

#include <mdspan>

enum class BlockType : u8
{
    Air,
    Grass,
    Dirt,
    Stone,
};

struct Chunk
{
    static constexpr glm::uvec3 size{ 16, 256, 16 };
    static constexpr u32 block_count = size.x * size.y * size.z;

    std::array<BlockType, block_count> blocks;

    auto view_3d() { return std::mdspan(blocks.data(), size.x, size.y, size.z); }
};

class ChunkGenerator
{
public:
    ChunkGenerator() = delete;

    static auto init(u32 seed) -> void;
    static auto generate(u32 chunk_x, u32 chunk_z) -> std::unique_ptr<Chunk>;

private:
    static inline u32 _seed = 0;

private:
    static auto get_height(glm::uvec2 world_position) -> u32;
};
