/**
 * @file hash.hpp
 * @brief Provides a hash specialization for glm::ivec2 to enable its use in hash-based containers.
 */

#pragma once

/**
 * @struct std::hash<glm::ivec2>
 * @brief Hash function specialization for glm::ivec2.
 *
 * Allows glm::ivec2 to be used as a key in hash-based containers such as std::unordered_map.
 */
template<> struct std::hash<glm::ivec2>
{
    /**
     * @brief Computes the hash value for a glm::ivec2.
     * @param vec The glm::ivec2 to hash.
     * @return The computed hash value.
     *
     * Combines the hash of the x and y components using a method similar to boost::hash_combine.
     */
    [[nodiscard]] auto operator()(const glm::ivec2& vec) const noexcept -> std::size_t
    {
        auto h1 = std::hash<i32>{}(vec.x);
        auto h2 = std::hash<i32>{}(vec.y);

        // Similar to boost::hash_combine.
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
