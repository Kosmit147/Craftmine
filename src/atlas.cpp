#include "atlas.hpp"

auto TextureAtlas::operator[](usize index) const -> TextureCoordinates
{
    auto row = index / _cols;
    auto col = index % _cols;
    ZTH_ASSERT(row < _rows && col < _cols);
    return operator[](row, col);
}

auto TextureAtlas::operator[](usize row, usize col) const -> TextureCoordinates
{
    ZTH_ASSERT(row < _rows && col < _cols);

    row = _rows - row - 1; // Have to reverse the row because OpenGL textures have 0 at the bottom on the y-axis.

    auto x1 = static_cast<float>(col) * _col_step;
    auto x2 = static_cast<float>(col + 1) * _col_step;
    auto y1 = static_cast<float>(row) * _row_step;
    auto y2 = static_cast<float>(row + 1) * _row_step;

    return TextureCoordinates{
        .top_left = glm::vec2{ x1, y1 },
        .bottom_left = glm::vec2{ x1, y2 },
        .bottom_right = glm::vec2{ x2, y2 },
        .top_right = glm::vec2{ x2, y1 },
    };
}
