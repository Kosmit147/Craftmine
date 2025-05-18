#pragma once

#include "fwd.hpp"

class WorldGenerator
{
public:
    static inline float scale = 0.015f;
    static inline float min_height = 0.3f;
    static inline float max_height = 0.45f;

public:
    WorldGenerator() = delete;

    [[nodiscard]] static auto generate(glm::ivec2 chunk_position) -> std::shared_ptr<ChunkData>;

private:
    [[nodiscard]] static auto noise(i32 world_x, i32 world_z) -> i32;
};
