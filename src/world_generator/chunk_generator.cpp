#include "chunk_generator.hpp"
#include <glm/glm.hpp>

ChunkGenerator::ChunkGenerator(int seed) : seed(seed) {}

float ChunkGenerator::get_height(float worldX, float worldZ) const
{
    glm::vec2 pos(worldX, worldZ);
    float noiseValue = zth::Random::perlin_noise<2, float>(pos);
    return noiseValue * 0.5f + 0.5f;
}

ChunkData ChunkGenerator::generate_chunk(int chunkX, int chunkZ)
{
    ChunkData chunk(CHUNK_WIDTH, std::vector<std::vector<BlockType>>(CHUNK_DEPTH, std::vector<BlockType>(CHUNK_HEIGHT, BlockType::Air)));

    float scale = 0.05f;
    int baseHeight = 64;

    for (int x = 0; x < CHUNK_WIDTH; ++x)
    {
        for (int z = 0; z < CHUNK_DEPTH; ++z)
        {
            float worldX = (chunkX * CHUNK_WIDTH + x) * scale;
            float worldZ = (chunkZ * CHUNK_DEPTH + z) * scale;

            float noise = get_height(worldX, worldZ);
            int height = baseHeight + static_cast<int>(noise * 30.0f);

            for (int y = 0; y <= height && y < CHUNK_HEIGHT; ++y)
            {
                if (y == height)
                    chunk[x][z][y] = BlockType::Grass;
                else if (y > height - 4)
                    chunk[x][z][y] = BlockType::Dirt;
                else
                    chunk[x][z][y] = BlockType::Stone;
            }
        }
    }

    return chunk;
}
