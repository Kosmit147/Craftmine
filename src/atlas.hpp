#pragma once

#include "quad.hpp"

using QuadTextureCoordinates = std::array<glm::vec2, vertices_per_quad>;

class TextureAtlas
{
public:
    constexpr explicit TextureAtlas(usize size) : TextureAtlas(size, size) {}
    constexpr explicit TextureAtlas(usize rows, usize cols)
        : _rows(rows), _cols(cols), _row_step(1.0f / static_cast<float>(_rows)),
          _col_step(1.0f / static_cast<float>(_cols))
    {}

    auto operator[](usize index) const -> QuadTextureCoordinates;
    auto operator[](usize row, usize col) const -> QuadTextureCoordinates;

private:
    usize _rows = 0;
    usize _cols = 0;

    float _row_step = 0.0f;
    float _col_step = 0.0f;
};
