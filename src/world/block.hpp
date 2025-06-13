/**
 * @file block.hpp
 * @brief Defines block types, block facing bitmask, and related utility functions for the world.
 */

#pragma once

/**
 * @enum BlockType
 * @brief Enumerates the types of blocks available in the world.
 */
enum class BlockType : u8
{
    Air = 0,   ///< Represents an empty or non-existent block.
    Grass,     ///< Represents a grass block.
    Dirt,      ///< Represents a dirt block.
    Stone,     ///< Represents a stone block.
};

/**
 * @enum BlockFacing
 * @brief Bitmask enumeration for block face directions.
 *
 * Used to specify which faces of a block are visible or interactable.
 */
enum BlockFacing : u8
{
    Facing_None = 0,        ///< No face.

    // Side faces.
    Facing_Backward = 1 << 0, ///< Backward face (+Z).
    Facing_Forward  = 1 << 1, ///< Forward face (-Z).
    Facing_Left     = 1 << 2, ///< Left face (-X).
    Facing_Right    = 1 << 3, ///< Right face (+X).

    // Up and down faces.
    Facing_Down = 1 << 4,     ///< Bottom face (-Y).
    Facing_Up   = 1 << 5,     ///< Top face (+Y).
};

// clang-format off

/**
 * @brief Bitwise OR assignment operator for BlockFacing.
 * @param lhs Left-hand side BlockFacing reference.
 * @param rhs Right-hand side BlockFacing value.
 * @return Reference to the result.
 */
constexpr auto operator|=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs | rhs); }

/**
 * @brief Bitwise AND assignment operator for BlockFacing.
 * @param lhs Left-hand side BlockFacing reference.
 * @param rhs Right-hand side BlockFacing value.
 * @return Reference to the result.
 */
constexpr auto operator&=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs & rhs); }

// clang-format on

/**
 * @brief Checks if the given BlockFacing value represents any side face (not up or down).
 * @param facing The BlockFacing value to check.
 * @return True if facing is a side face, false otherwise.
 */
[[nodiscard]] constexpr auto is_side_facing(BlockFacing facing) -> bool
{
    constexpr auto side_faces = Facing_Backward | Facing_Forward | Facing_Left | Facing_Right;
    return facing & side_faces;
}
