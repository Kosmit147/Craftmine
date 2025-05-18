#include "world/generator.hpp"

#include "world/chunk.hpp"

auto WorldGenerator::generate(glm::ivec2 chunk_position) -> std::shared_ptr<ChunkData>
{
    // @multithreaded

    auto chunk_data = std::make_shared_for_overwrite<ChunkData>();

    auto [chunk_pos_x, chunk_pos_z] = chunk_position;
    auto chunk_start_x = chunk_x_to_world_x(chunk_pos_x);
    auto chunk_start_z = chunk_z_to_world_z(chunk_pos_z);

    // @speed: We're iterating over the blocks in a not cache-efficient way.
    for (i32 x = 0; x < chunk_size.x; x++)
    {
        for (i32 z = 0; z < chunk_size.z; z++)
        {
            auto height = noise(chunk_start_x + x, chunk_start_z + z);

            for (i32 y = 0; y < chunk_size.y; y++)
            {
                glm::ivec3 coords{ x, y, z };
                auto& data = *chunk_data;

                if (y > height)
                    data[coords] = BlockType::Air;
                else if (y == height)
                    data[coords] = BlockType::Grass;
                else if (y > height - 4)
                    data[coords] = BlockType::Dirt;
                else
                    data[coords] = BlockType::Stone;
            }
        }
    }

    return chunk_data;
}

auto WorldGenerator::noise(i32 world_x, i32 world_z) -> i32
{
    glm::vec2 position{ world_x, world_z };
    position *= scale;

    auto noise = zth::Random::perlin_noise(position) * 0.5f + 0.5f;
    auto height = std::lerp(min_height, max_height, noise);

    return static_cast<i32>(height * static_cast<float>(chunk_size.y));
}
