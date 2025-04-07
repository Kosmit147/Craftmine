#include "chunk.hpp"

auto ChunkGenerator::init(u32 seed) -> void
{
    _seed = seed;
}

auto ChunkGenerator::generate(u32 chunk_x, u32 chunk_z) -> std::unique_ptr<Chunk>
{
    auto chunk = std::make_unique<Chunk>();
    auto& chunk_size = Chunk::size;

    auto world_x = chunk_x * chunk_size.x;
    auto world_z = chunk_z * chunk_size.z;

    for (u32 x = 0; x < chunk_size.x; x++)
    {
        for (u32 z = 0; z < chunk_size.z; z++)
        {
            glm::uvec2 block_position{ world_x + x, world_z + z };
            auto height = get_height(block_position);

            for (u32 y = 0; y < chunk_size.y; y++)
            {
                // @todo: Refactor this loop.

                auto view = chunk->view_3d();
                assert(height > 4);

                if (y > height)
                    view[x, y, z] = BlockType::Air;
                else if (y == height)
                    view[x, y, z] = BlockType::Grass;
                else if (y > height - 4)
                    view[x, y, z] = BlockType::Dirt;
                else
                    view[x, y, z] = BlockType::Stone;
            }
        }
    }

    return chunk;
}

auto ChunkGenerator::get_height(glm::uvec2 world_position) -> u32
{
    constexpr auto scale = 0.01f;
    constexpr auto min_height = 0.2f;
    constexpr auto max_height = 0.45f;

    glm::vec2 position = world_position;
    position *= scale;

    auto noise = zth::Random::perlin_noise(position) * 0.5f + 0.5f;
    auto height = std::lerp(min_height, max_height, noise);

    return static_cast<u32>(height * static_cast<float>(Chunk::size.y));
}
