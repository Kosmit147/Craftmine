#include "chunk.hpp"

#include "atlas.hpp"

namespace {

struct Face
{
    std::array<glm::vec3, zth::vertices_per_quad> vertices;
    glm::vec3 normal;
};

// clang-format off

constexpr Face backward_face = {
    .vertices = {
        glm::vec3{ 0.0f, 1.0f, 1.0f }, // Top-left.
        glm::vec3{ 0.0f, 0.0f, 1.0f }, // Bottom-left.
        glm::vec3{ 1.0f, 0.0f, 1.0f }, // Bottom-right.
        glm::vec3{ 1.0f, 1.0f, 1.0f }, // Top-right.
    },
    .normal = zth::math::world_backward,
};

constexpr Face forward_face = {
    .vertices = {
        glm::vec3{ 1.0f, 1.0f, 0.0f }, // Top-left.
        glm::vec3{ 1.0f, 0.0f, 0.0f }, // Bottom-left.
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // Bottom-right.
        glm::vec3{ 0.0f, 1.0f, 0.0f }, // Top-right.
    },
    .normal = zth::math::world_forward,
};

constexpr Face left_face = {
    .vertices = {
        glm::vec3{ 0.0f, 1.0f, 0.0f }, // Top-left.
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // Bottom-left.
        glm::vec3{ 0.0f, 0.0f, 1.0f }, // Bottom-right.
        glm::vec3{ 0.0f, 1.0f, 1.0f }, // Top-right.
    },
    .normal = zth::math::world_left,
};

constexpr Face right_face = {
    .vertices = {
        glm::vec3{ 1.0f, 1.0f, 1.0f }, // Top-left.
        glm::vec3{ 1.0f, 0.0f, 1.0f }, // Bottom-left.
        glm::vec3{ 1.0f, 0.0f, 0.0f }, // Bottom-right.
        glm::vec3{ 1.0f, 1.0f, 0.0f }, // Top-right.
    },
    .normal = zth::math::world_right,
};

constexpr Face down_face = {
    .vertices = {
        glm::vec3{ 0.0f, 0.0f, 1.0f }, // Top-left.
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // Bottom-left.
        glm::vec3{ 1.0f, 0.0f, 0.0f }, // Bottom-right.
        glm::vec3{ 1.0f, 0.0f, 1.0f }, // Top-right.
    },
    .normal = zth::math::world_down,
};

constexpr Face up_face = {
    .vertices = {
        glm::vec3{ 0.0f, 1.0f, 0.0f }, // Top-left.
        glm::vec3{ 0.0f, 1.0f, 1.0f }, // Bottom-left.
        glm::vec3{ 1.0f, 1.0f, 1.0f }, // Bottom-right.
        glm::vec3{ 1.0f, 1.0f, 0.0f }, // Top-right.
    },
    .normal = zth::math::world_up,
};

// clang-format on

constexpr TextureAtlas blocks_texture_atlas{ 4, 4 };

[[nodiscard]] auto get_block_texture_index(BlockType block, BlockFacing facing) -> usize
{
    switch (block)
    {
        using enum BlockType;
    case Grass:
        if (is_side_facing(facing))
            return 1;
        return 2;
    case Dirt:
        return 3;
    case Stone:
        return 0;
    }

    ZTH_ASSERT(false);
    std::unreachable();
}

[[nodiscard]] auto get_face(BlockFacing facing) -> const Face&
{
    switch (facing)
    {
    case Facing_Backward:
        return backward_face;
    case Facing_Forward:
        return forward_face;
    case Facing_Left:
        return left_face;
    case Facing_Right:
        return right_face;
    case Facing_Down:
        return down_face;
    case Facing_Up:
        return up_face;
    }

    ZTH_ASSERT(false);
    std::unreachable();
}

auto append_single_face_vertices(zth::Vector<zth::StandardVertex>& vertices, BlockType block, BlockFacing facing,
                                 glm::ivec3 coordinates) -> void
{
    auto tex_coords = blocks_texture_atlas[get_block_texture_index(block, facing)];
    auto& face = get_face(facing);

    for (usize i = 0; i < zth::vertices_per_quad; i++)
    {
        vertices.push_back(zth::StandardVertex{
            .position = face.vertices[i] + glm::vec3{ coordinates },
            .normal = face.normal,
            .uv = tex_coords[i],
        });
    }
}

auto append_block_vertices(zth::Vector<zth::StandardVertex>& vertices, BlockType block, BlockFacing facing,
                           glm::ivec3 coordinates) -> void
{
    if (block == BlockType::Air)
        return;

    if (facing & Facing_Backward)
        append_single_face_vertices(vertices, block, Facing_Backward, coordinates);

    if (facing & Facing_Forward)
        append_single_face_vertices(vertices, block, Facing_Forward, coordinates);

    if (facing & Facing_Left)
        append_single_face_vertices(vertices, block, Facing_Left, coordinates);

    if (facing & Facing_Right)
        append_single_face_vertices(vertices, block, Facing_Right, coordinates);

    if (facing & Facing_Down)
        append_single_face_vertices(vertices, block, Facing_Down, coordinates);

    if (facing & Facing_Up)
        append_single_face_vertices(vertices, block, Facing_Up, coordinates);
}

} // namespace

auto ChunkData::at(glm::ivec3 coordinates) -> Optional<Reference<BlockType>>
{
    if (!valid_coordinates(coordinates))
        return nil;

    return operator[](coordinates);
}

auto ChunkData::at(glm::ivec3 coordinates) const -> Optional<Reference<const BlockType>>
{
    if (!valid_coordinates(coordinates))
        return nil;

    return operator[](coordinates);
}

auto ChunkData::at_exterior(glm::ivec3 coordinates, const NeighborsArray& neighbors) const
    -> Optional<Reference<const BlockType>>
{
    auto [x, y, z] = coordinates;

    if (x == -1)
    {
        if (!neighbors[minus_x_idx])
            return nil;

        return neighbors[minus_x_idx]->operator[]({ chunk_size.x - 1, y, z });
    }

    if (x == chunk_size.x)
    {
        if (!neighbors[plus_x_idx])
            return nil;

        return neighbors[plus_x_idx]->operator[]({ 0, y, z });
    }

    if (z == -1)
    {
        if (!neighbors[minus_z_idx])
            return nil;

        return neighbors[minus_z_idx]->operator[]({ x, y, chunk_size.z - 1 });
    }

    if (z == chunk_size.z)
    {
        if (!neighbors[plus_z_idx])
            return nil;

        return neighbors[plus_z_idx]->operator[]({ x, y, 0 });
    }

    if (y == -1 || y == chunk_size.y)
        return nil;

    ZTH_ASSERT(valid_coordinates(coordinates));
    return operator[](coordinates);
}

auto ChunkData::operator[](glm::ivec3 coordinates) -> BlockType&
{
    ZTH_ASSERT(valid_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    std::mdspan view{ _data.data(), chunk_size.x, chunk_size.y, chunk_size.z };
    return view[x, y, z];
}

auto ChunkData::operator[](glm::ivec3 coordinates) const -> const BlockType&
{
    ZTH_ASSERT(valid_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    std::mdspan view{ _data.data(), chunk_size.x, chunk_size.y, chunk_size.z };
    return view[x, y, z];
}

auto ChunkData::generate_mesh(const NeighborsArray& neighbors) const -> zth::Vector<zth::StandardVertex>
{
    // @multithreaded

    zth::Vector<zth::StandardVertex> result;
    // @speed: Check if reserving some space for the vertices here would be good.

    append_vertices_for_exterior_blocks(result, neighbors);
    append_vertices_for_interior_blocks(result);

    return result;
}

auto ChunkData::valid_coordinates(glm::ivec3 coordinates) -> bool
{
    auto [x, y, z] = coordinates;

    auto valid_x = x >= 0 && x < chunk_size.x;
    auto valid_y = y >= 0 && y < chunk_size.y;
    auto valid_z = z >= 0 && z < chunk_size.z;

    if (valid_x && valid_y && valid_z)
        return true;

    return false;
}

auto ChunkData::visible_faces_for_exterior_block(glm::ivec3 coordinates, const NeighborsArray& neighbors) const
    -> BlockFacing
{
    ZTH_ASSERT(exterior_block_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    auto facing = Facing_None;

    auto backward = at_exterior({ x, y, z + 1 }, neighbors);
    auto forward = at_exterior({ x, y, z - 1 }, neighbors);
    auto left = at_exterior({ x - 1, y, z }, neighbors);
    auto right = at_exterior({ x + 1, y, z }, neighbors);
    auto down = at_exterior({ x, y - 1, z }, neighbors);
    auto up = at_exterior({ x, y + 1, z }, neighbors);

    if (!backward || backward->get() == BlockType::Air)
        facing |= Facing_Backward;

    if (!forward || forward->get() == BlockType::Air)
        facing |= Facing_Forward;

    if (!left || left->get() == BlockType::Air)
        facing |= Facing_Left;

    if (!right || right->get() == BlockType::Air)
        facing |= Facing_Right;

    if (!down || down->get() == BlockType::Air)
        facing |= Facing_Down;

    if (!up || up->get() == BlockType::Air)
        facing |= Facing_Up;

    return facing;
}

auto ChunkData::visible_faces_for_interior_block(glm::ivec3 coordinates) const -> BlockFacing
{
    ZTH_ASSERT(interior_block_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    auto facing = Facing_None;

    auto backward = operator[]({ x, y, z + 1 });
    auto forward = operator[]({ x, y, z - 1 });
    auto left = operator[]({ x - 1, y, z });
    auto right = operator[]({ x + 1, y, z });
    auto down = operator[]({ x, y - 1, z });
    auto up = operator[]({ x, y + 1, z });

    if (backward == BlockType::Air)
        facing |= Facing_Backward;

    if (forward == BlockType::Air)
        facing |= Facing_Forward;

    if (left == BlockType::Air)
        facing |= Facing_Left;

    if (right == BlockType::Air)
        facing |= Facing_Right;

    if (down == BlockType::Air)
        facing |= Facing_Down;

    if (up == BlockType::Air)
        facing |= Facing_Up;

    return facing;
}

auto ChunkData::append_vertices_for_exterior_blocks(zth::Vector<zth::StandardVertex>& vertices,
                                                    const NeighborsArray& neighbors) const -> void
{
    // Backward-facing wall (grasps parts of left, right, top and bottom walls as well).
    // +z
    {
        constexpr auto xs = std::views::iota(0, chunk_size.x);
        constexpr auto ys = std::views::iota(0, chunk_size.y);
        constexpr auto zs = std::views::iota(chunk_size.z - 1, chunk_size.z);
        constexpr auto wall = std::views::cartesian_product(xs, ys, zs);
        append_vertices_for_wall(vertices, wall, neighbors);
    }

    // Forward-facing wall (grasps parts of left, right, top and bottom walls as well).
    // -z
    {
        constexpr auto xs = std::views::iota(0, chunk_size.x);
        constexpr auto ys = std::views::iota(0, chunk_size.y);
        constexpr auto zs = std::views::iota(0, 1);
        constexpr auto wall = std::views::cartesian_product(xs, ys, zs);
        append_vertices_for_wall(vertices, wall, neighbors);
    }

    // Left-facing wall (grasps parts of top and bottom walls as well).
    // -x
    {
        constexpr auto xs = std::views::iota(0, 1);
        constexpr auto ys = std::views::iota(0, chunk_size.y);
        constexpr auto zs = std::views::iota(1, chunk_size.z - 1);
        constexpr auto wall = std::views::cartesian_product(xs, ys, zs);
        append_vertices_for_wall(vertices, wall, neighbors);
    }

    // Right-facing wall (grasps parts of top and bottom walls as well).
    // +x
    {
        constexpr auto xs = std::views::iota(chunk_size.x - 1, chunk_size.x);
        constexpr auto ys = std::views::iota(0, chunk_size.y);
        constexpr auto zs = std::views::iota(1, chunk_size.z - 1);
        constexpr auto wall = std::views::cartesian_product(xs, ys, zs);
        append_vertices_for_wall(vertices, wall, neighbors);
    }

    // Bottom-facing wall.
    // -y
    {
        constexpr auto xs = std::views::iota(1, chunk_size.x - 1);
        constexpr auto ys = std::views::iota(0, 1);
        constexpr auto zs = std::views::iota(1, chunk_size.z - 1);
        constexpr auto wall = std::views::cartesian_product(xs, ys, zs);
        append_vertices_for_wall(vertices, wall, neighbors);
    }

    // Top-facing wall.
    // +y
    {
        constexpr auto xs = std::views::iota(1, chunk_size.x - 1);
        constexpr auto ys = std::views::iota(chunk_size.y - 1, chunk_size.y);
        constexpr auto zs = std::views::iota(1, chunk_size.z - 1);
        constexpr auto wall = std::views::cartesian_product(xs, ys, zs);
        append_vertices_for_wall(vertices, wall, neighbors);
    }
}

auto ChunkData::append_vertices_for_interior_blocks(zth::Vector<zth::StandardVertex>& vertices) const -> void
{
    // We don't need to worry about whether the coordinates of neighboring blocks are correct as we're only handling the
    // interior blocks.

    // Skip the exterior blocks.
    constexpr auto xs = std::views::iota(1, chunk_size.x - 1);
    constexpr auto ys = std::views::iota(1, chunk_size.y - 1);
    constexpr auto zs = std::views::iota(1, chunk_size.z - 1);
    constexpr auto coords = std::views::cartesian_product(xs, ys, zs);

    for (const auto [x, y, z] : coords)
    {
        glm::ivec3 block_coords{ x, y, z };
        auto block = operator[](block_coords);
        auto facing = visible_faces_for_interior_block(block_coords);
        append_block_vertices(vertices, block, facing, block_coords);
    }
}

auto ChunkData::append_vertices_for_wall(zth::Vector<zth::StandardVertex>& vertices, const WallCoordinates& wall,
                                         const NeighborsArray& neighbors) const -> void
{
    for (const auto [x, y, z] : wall)
    {
        glm::ivec3 block_coords{ x, y, z };
        auto block = operator[](block_coords);
        auto facing = visible_faces_for_exterior_block(block_coords, neighbors);
        append_block_vertices(vertices, block, facing, block_coords);
    }
}

auto ChunkData::exterior_block_coordinates(glm::ivec3 coordinates) -> bool
{
    auto [x, y, z] = coordinates;
    return exterior_x(x) || exterior_y(y) || exterior_z(z);
}

auto ChunkData::exterior_x(i32 x) -> bool
{
    return x == 0 || x == chunk_size.x - 1;
}

auto ChunkData::exterior_y(i32 y) -> bool
{
    return y == 0 || y == chunk_size.y - 1;
}

auto ChunkData::exterior_z(i32 z) -> bool
{
    return z == 0 || z == chunk_size.z - 1;
}

auto ChunkData::interior_block_coordinates(glm::ivec3 coordinates) -> bool
{
    auto [x, y, z] = coordinates;
    return interior_x(x) && interior_y(y) && interior_z(z);
}

auto ChunkData::interior_x(i32 x) -> bool
{
    return x > 0 && x < chunk_size.x - 1;
}

auto ChunkData::interior_y(i32 y) -> bool
{
    return y > 0 && y < chunk_size.y - 1;
}

auto ChunkData::interior_z(i32 z) -> bool
{
    return z > 0 && z < chunk_size.z - 1;
}

auto world_x_to_chunk_x(i32 x) -> i32
{
    return x / chunk_size.x;
}

auto world_z_to_chunk_z(i32 z) -> i32
{
    return z / chunk_size.z;
}

auto chunk_x_to_world_x(i32 x) -> i32
{
    return x * chunk_size.x;
}

auto chunk_z_to_world_z(i32 z) -> i32
{
    return z * chunk_size.z;
}
