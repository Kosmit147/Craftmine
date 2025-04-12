#pragma once

struct TextureCoordinates
{
    glm::vec2 top_left;     // 0
    glm::vec2 bottom_left;  // 1
    glm::vec2 bottom_right; // 2
    glm::vec2 top_right;    // 3

    auto operator[](usize index) const -> glm::vec2
    {
        ZTH_ASSERT(index <= 3);
        return (&top_left)[index];
    }
};

class TextureAtlas
{
public:
    constexpr explicit TextureAtlas(usize size) : TextureAtlas(size, size) {}
    constexpr explicit TextureAtlas(usize rows, usize cols)
        : _rows(rows), _cols(cols), _row_step(1.0f / static_cast<float>(_rows)),
          _col_step(1.0f / static_cast<float>(_cols))
    {}

    auto operator[](usize index) const -> TextureCoordinates;
    auto operator[](usize row, usize col) const -> TextureCoordinates;

private:
    usize _rows = 0;
    usize _cols = 0;

    float _row_step = 0.0f;
    float _col_step = 0.0f;
};
