#pragma once

template<> struct std::hash<glm::ivec2>
{
    [[nodiscard]] auto operator()(const glm::ivec2& vec) const noexcept -> std::size_t
    {
        auto h1 = std::hash<i32>{}(vec.x);
        auto h2 = std::hash<i32>{}(vec.y);

        // Similar to boost::hash_combine.
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
