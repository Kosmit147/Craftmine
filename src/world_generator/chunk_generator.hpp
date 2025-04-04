#pragma once

#include <cstdint>
#include <vector>

constexpr int chunk_width = 16;
constexpr int chunk_height = 256;
constexpr int chunk_depth = 16;

enum class BlockType : uint8_t
{
    Air,
    Grass,
    Dirt,
    Stone
};

using ChunkData = std::vector<std::vector<std::vector<BlockType>>>;

class ChunkGenerator
{
public:
    ChunkGenerator(int seed = 0);
    ChunkData generate_chunk(int chunkX, int chunkZ);

private:
    int seed;
    float get_height(float worldX, float worldZ) const;
};
