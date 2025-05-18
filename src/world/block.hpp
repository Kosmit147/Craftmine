#pragma once

enum class BlockType : u8
{
    Air = 0,
    Grass,
    Dirt,
    Stone,
};

// This is a bitmask type.
enum BlockFacing : u8
{
    Facing_None = 0,

    // Side faces.
    Facing_Backward = 1 << 0,
    Facing_Forward = 1 << 1,
    Facing_Left = 1 << 2,
    Facing_Right = 1 << 3,

    // Up and down faces.
    Facing_Down = 1 << 4,
    Facing_Up = 1 << 5,
};

// clang-format off
constexpr auto operator|=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs | rhs); }
constexpr auto operator&=(BlockFacing& lhs, BlockFacing rhs) -> BlockFacing& { return lhs = static_cast<BlockFacing>(lhs & rhs); }
// clang-format on

[[nodiscard]] constexpr auto is_side_facing(BlockFacing facing) -> bool
{
    constexpr auto side_faces = Facing_Backward | Facing_Forward | Facing_Left | Facing_Right;
    return facing & side_faces;
}
