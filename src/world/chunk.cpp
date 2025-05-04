#include "chunk.hpp"

#include <glm/gtx/structured_bindings.hpp>

#include "atlas.hpp"
#include "quad.hpp"

namespace {

struct Face
{
    std::array<glm::vec3, vertices_per_quad> vertices;
    glm::vec3 normal;
};

// clang-format off

constexpr Face backward_face = {
    .vertices = {
        glm::vec3{ 0.0f, 1.0f, 1.0f }, // top-left
        glm::vec3{ 0.0f, 0.0f, 1.0f }, // bottom-left
        glm::vec3{ 1.0f, 0.0f, 1.0f }, // bottom-right
        glm::vec3{ 1.0f, 1.0f, 1.0f }, // top-right
    },
    .normal = zth::math::world_backward,
};

constexpr Face forward_face = {
    .vertices = {
        glm::vec3{ 1.0f, 1.0f, 0.0f }, // top-left
        glm::vec3{ 1.0f, 0.0f, 0.0f }, // bottom-left
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // bottom-right
        glm::vec3{ 0.0f, 1.0f, 0.0f }, // top-right
    },
    .normal = zth::math::world_forward,
};

constexpr Face left_face = {
    .vertices = {
        glm::vec3{ 0.0f, 1.0f, 0.0f }, // top-left
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // bottom-left
        glm::vec3{ 0.0f, 0.0f, 1.0f }, // bottom-right
        glm::vec3{ 0.0f, 1.0f, 1.0f }, // top-right
    },
    .normal = zth::math::world_left,
};

constexpr Face right_face = {
    .vertices = {
        glm::vec3{ 1.0f, 1.0f, 1.0f }, // top-left
        glm::vec3{ 1.0f, 0.0f, 1.0f }, // bottom-left
        glm::vec3{ 1.0f, 0.0f, 0.0f }, // bottom-right
        glm::vec3{ 1.0f, 1.0f, 0.0f }, // top-right
    },
    .normal = zth::math::world_right,
};

constexpr Face down_face = {
    .vertices = {
        glm::vec3{ 0.0f, 0.0f, 1.0f }, // top-left
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // bottom-left
        glm::vec3{ 1.0f, 0.0f, 0.0f }, // bottom-right
        glm::vec3{ 1.0f, 0.0f, 1.0f }, // top-right
    },
    .normal = zth::math::world_down,
};

constexpr Face up_face = {
    .vertices = {
        glm::vec3{ 0.0f, 1.0f, 0.0f }, // top-left
        glm::vec3{ 0.0f, 1.0f, 1.0f }, // bottom-left
        glm::vec3{ 1.0f, 1.0f, 1.0f }, // bottom-right
        glm::vec3{ 1.0f, 1.0f, 0.0f }, // top-right
    },
    .normal = zth::math::world_up,
};

// clang-format on

constexpr TextureAtlas blocks_texture_atlas{ 4, 4 };

[[nodiscard]] auto get_block_texture_index(Block block, BlockFacing facing) -> usize
{
    switch (block)
    {
        using enum Block;
    case Grass:
        if (side_facing(facing))
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

auto append_single_face_vertices(zth::Vector<zth::StandardVertex>& vertices, Block block, BlockFacing facing,
                                 glm::ivec3 coordinates) -> void
{
    auto tex_coords = blocks_texture_atlas[get_block_texture_index(block, facing)];
    auto& face = get_face(facing);

    for (usize i = 0; i < vertices_per_quad; i++)
    {
        vertices.push_back(zth::StandardVertex{
            .local_position = face.vertices[i] + glm::vec3{ coordinates },
            .normal = face.normal,
            .tex_coords = tex_coords[i],
        });
    }
}

auto append_block_vertices(zth::Vector<zth::StandardVertex>& vertices, Block block, BlockFacing facing,
                           glm::ivec3 coordinates) -> void
{
    if (block == Block::Air)
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

auto side_facing(BlockFacing facing) -> bool
{
    constexpr auto side_faces = Facing_Backward | Facing_Forward | Facing_Left | Facing_Right;
    return facing & side_faces;
}

ChunkData::ChunkData(zth::UniquePtr<BlocksArray>&& data) : _data{ std::move(data) } {}

auto ChunkData::at(glm::ivec3 coordinates) -> Optional<Reference<Block>>
{
    if (!valid_coordinates(coordinates))
        return nil;

    return operator[](coordinates);
}

auto ChunkData::at(glm::ivec3 coordinates) const -> Optional<Reference<const Block>>
{
    if (!valid_coordinates(coordinates))
        return nil;

    return operator[](coordinates);
}

auto ChunkData::operator[](glm::ivec3 coordinates) -> Block&
{
    ZTH_ASSERT(valid_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    std::mdspan view{ _data->data(), chunk_size.x, chunk_size.y, chunk_size.z };
    return view[x, y, z];
}

auto ChunkData::operator[](glm::ivec3 coordinates) const -> const Block&
{
    ZTH_ASSERT(valid_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    std::mdspan view{ _data->data(), chunk_size.x, chunk_size.y, chunk_size.z };
    return view[x, y, z];
}

auto ChunkData::generate_mesh() const -> zth::Vector<zth::StandardVertex>
{
    // @multithreaded

    zth::Vector<zth::StandardVertex> result;
    // @speed: Check if reserving some space for the vertices here would be good.

    constexpr auto xs = std::views::iota(0, chunk_size.x);
    constexpr auto ys = std::views::iota(0, chunk_size.y);
    constexpr auto zs = std::views::iota(0, chunk_size.z);
    constexpr auto coords = std::views::cartesian_product(xs, ys, zs);

    for (const auto [x, y, z] : coords)
    {
        glm::ivec3 block_coords{ x, y, z };
        auto block = operator[](block_coords);
        auto facing = visible_faces_for_block(block_coords);
        append_block_vertices(result, block, facing, block_coords);
    }

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

auto ChunkData::visible_faces_for_block(glm::ivec3 coordinates) const -> BlockFacing
{
    auto [x, y, z] = coordinates;
    auto facing = Facing_None;

    auto backward = at(glm::vec3{ x, y, z + 1 });
    auto forward = at(glm::vec3{ x, y, z - 1 });
    auto left = at(glm::vec3{ x - 1, y, z });
    auto right = at(glm::vec3{ x + 1, y, z });
    auto down = at(glm::vec3{ x, y - 1, z });
    auto up = at(glm::vec3{ x, y + 1, z });

    if (!backward || backward->get() == Block::Air)
        facing |= Facing_Backward;

    if (!forward || forward->get() == Block::Air)
        facing |= Facing_Forward;

    if (!left || left->get() == Block::Air)
        facing |= Facing_Left;

    if (!right || right->get() == Block::Air)
        facing |= Facing_Right;

    if (!down || down->get() == Block::Air)
        facing |= Facing_Down;

    if (!up || up->get() == Block::Air)
        facing |= Facing_Up;

    return facing;
}

Chunk::Chunk(ChunkData&& data) : _data{ std::move(data) } {}

auto Chunk::upload_mesh(const zth::Vector<zth::StandardVertex>& mesh) -> void
{
    _mesh = std::make_shared<zth::QuadMesh>(mesh, zth::StandardVertex::layout);
}

auto Chunk::to_world_x(i32 x) -> i32
{
    return x * chunk_size.x;
}

auto Chunk::to_world_z(i32 z) -> i32
{
    return z * chunk_size.z;
}

auto ChunkGenerator::generate(ChunkPosition position) -> ChunkData
{
    // @multithreaded

    ChunkData chunk_data;

    auto chunk_start_x = Chunk::to_world_x(position.x);
    auto chunk_start_z = Chunk::to_world_z(position.z);

    // @speed: We're iterating over the blocks in a not cache-efficient way.
    for (i32 x = 0; x < chunk_size.x; x++)
    {
        for (i32 z = 0; z < chunk_size.z; z++)
        {
            auto height = get_height(chunk_start_x + x, chunk_start_z + z);

            for (i32 y = 0; y < chunk_size.y; y++)
            {
                glm::ivec3 coords{ x, y, z };

                if (y > height)
                    chunk_data[coords] = Block::Air;
                else if (y == height)
                    chunk_data[coords] = Block::Grass;
                else if (y > height - 4)
                    chunk_data[coords] = Block::Dirt;
                else
                    chunk_data[coords] = Block::Stone;
            }
        }
    }

    return chunk_data;
}

auto ChunkGenerator::get_height(i32 world_x, i32 world_z) -> i32
{
    glm::vec2 position{ world_x, world_z };
    position *= scale;

    auto noise = zth::Random::perlin_noise(position) * 0.5f + 0.5f;
    auto height = std::lerp(min_height, max_height, noise);

    return static_cast<i32>(height * static_cast<float>(chunk_size.y));
}
