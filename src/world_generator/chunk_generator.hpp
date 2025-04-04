#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

constexpr int CHUNK_WIDTH = 16;
constexpr int CHUNK_HEIGHT = 256;
constexpr int CHUNK_DEPTH = 16;

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

#endif // CHUNK_GENERATOR_H
