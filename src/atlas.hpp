/**
 * @file atlas.hpp
 * @brief Declares the TextureAtlas class for managing texture coordinates in a texture atlas.
 */

#pragma once

/**
 * @brief Type alias for an array of texture coordinates for a quad.
 */
using QuadTextureCoordinates = std::array<glm::vec2, zth::vertices_per_quad>;

/**
 * @class TextureAtlas
 * @brief Represents a 2D texture atlas for mapping indices to quad texture coordinates.
 *
 * Provides methods to retrieve the texture coordinates for a given index or row/column in the atlas.
 */
class TextureAtlas
{
public:
    /**
     * @brief Constructs a square texture atlas with the given size.
     * @param size The number of rows and columns (atlas is size x size).
     */
    constexpr explicit TextureAtlas(usize size) : TextureAtlas(size, size) {}

    /**
     * @brief Constructs a texture atlas with the given number of rows and columns.
     * @param rows The number of rows in the atlas.
     * @param cols The number of columns in the atlas.
     */
    constexpr explicit TextureAtlas(usize rows, usize cols)
        : _rows(rows), _cols(cols), _row_step(1.0f / static_cast<float>(_rows)),
          _col_step(1.0f / static_cast<float>(_cols))
    {}

    /**
     * @brief Retrieves the quad texture coordinates for the given linear index.
     * @param index The linear index in the atlas.
     * @return The quad texture coordinates.
     */
    [[nodiscard]] auto operator[](usize index) const -> QuadTextureCoordinates;

    /**
     * @brief Retrieves the quad texture coordinates for the given row and column.
     * @param row The row index in the atlas.
     * @param col The column index in the atlas.
     * @return The quad texture coordinates.
     */
    [[nodiscard]] auto operator[](usize row, usize col) const -> QuadTextureCoordinates;

private:
    /**
     * @brief The number of rows in the atlas.
     */
    usize _rows = 0;
    /**
     * @brief The number of columns in the atlas.
     */
    usize _cols = 0;

    /**
     * @brief The step size for each row (in normalized texture coordinates).
     */
    float _row_step = 0.0f;
    /**
     * @brief The step size for each column (in normalized texture coordinates).
     */
    float _col_step = 0.0f;
};
