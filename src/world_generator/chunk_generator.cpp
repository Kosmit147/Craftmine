#include "chunk_generator.hpp"

ChunkGenerator::ChunkGenerator(int seed) : seed(seed) {}

float ChunkGenerator::get_height(float worldX, float worldZ) const
{
    glm::vec2 pos{ worldX, worldZ };
    float noiseValue = zth::Random::perlin_noise(pos);
    return noiseValue * 0.5f + 0.5f;
}

ChunkData ChunkGenerator::generate_chunk(int chunkX, int chunkZ)
{
    ChunkData chunk(chunk_width, std::vector(chunk_depth, std::vector(chunk_height, BlockType::Air)));

    float scale = 0.05f;
    int baseHeight = 64;

    for (int x = 0; x < chunk_width; ++x)
    {
        for (int z = 0; z < chunk_depth; ++z)
        {
            float worldX = (chunkX * chunk_width + x) * scale;
            float worldZ = (chunkZ * chunk_depth + z) * scale;

            float noise = get_height(worldX, worldZ);
            int height = baseHeight + static_cast<int>(noise * 30.0f);

            for (int y = 0; y <= height && y < chunk_height; ++y)
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
