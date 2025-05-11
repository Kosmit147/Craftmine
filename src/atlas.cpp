#include "atlas.hpp"

auto TextureAtlas::operator[](usize index) const -> QuadTextureCoordinates
{
    auto row = index / _cols;
    auto col = index % _cols;
    ZTH_ASSERT(row < _rows && col < _cols);
    return operator[](row, col);
}

auto TextureAtlas::operator[](usize row, usize col) const -> QuadTextureCoordinates
{
    ZTH_ASSERT(row < _rows && col < _cols);

    row = _rows - row - 1; // Have to reverse the row because OpenGL textures have 0 at the bottom on the y-axis.

    auto x1 = static_cast<float>(col) * _col_step;
    auto x2 = static_cast<float>(col + 1) * _col_step;
    auto y1 = static_cast<float>(row + 1) * _row_step;
    auto y2 = static_cast<float>(row) * _row_step;

    return QuadTextureCoordinates{
        glm::vec2{ x1, y1 }, // top-left
        glm::vec2{ x1, y2 }, // bottom-left
        glm::vec2{ x2, y2 }, // bottom-right
        glm::vec2{ x2, y1 }, // top-right
    };
}
