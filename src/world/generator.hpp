/**
 * @file generator.hpp
 * @brief Declares the WorldGenerator class for procedural chunk generation.
 */

#pragma once

#include "fwd.hpp"

/**
 * @class WorldGenerator
 * @brief Provides static methods for generating world chunks procedurally.
 *
 * Contains parameters for terrain generation and utility functions for noise-based height calculation.
 */
class WorldGenerator
{
public:
    /**
     * @brief Scale factor for terrain noise.
     */
    static inline float scale = 0.015f;

    /**
     * @brief Minimum normalized height for terrain generation.
     */
    static inline float min_height = 0.3f;

    /**
     * @brief Maximum normalized height for terrain generation.
     */
    static inline float max_height = 0.45f;

public:
    /**
     * @brief Deleted default constructor to prevent instantiation.
     */
    WorldGenerator() = delete;

    /**
     * @brief Generates a chunk of world data at the given chunk position.
     * @param chunk_position The position of the chunk in chunk coordinates.
     * @return Shared pointer to the generated ChunkData.
     */
    [[nodiscard]] static auto generate(glm::ivec2 chunk_position) -> std::shared_ptr<ChunkData>;

private:
    /**
     * @brief Computes the noise value for a given world (x, z) coordinate.
     * @param world_x The world x coordinate.
     * @param world_z The world z coordinate.
     * @return The computed noise value as an integer.
     */
    [[nodiscard]] static auto noise(i32 world_x, i32 world_z) -> i32;
};
