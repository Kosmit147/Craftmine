#include "chunk.hpp"

#include <glm/gtx/structured_bindings.hpp>

#include "atlas.hpp"
#include "quad.hpp"

namespace {

using QuadVertices = std::array<glm::vec3, vertices_per_quad>;

struct Face
{
    QuadVertices vertices;
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

auto get_block_texture_index(Block block, BlockFacing facing) -> usize
{
    (void)facing; // @todo: Facing.

    switch (block)
    {
        using enum Block;
    case Grass:
        if (facing == Facing_Backward || facing == Facing_Forward || facing == Facing_Left || facing == Facing_Right)
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

auto get_face(BlockFacing facing) -> const Face&
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

auto append_single_face_vertices(ChunkMesh& mesh, Block block, BlockFacing facing, glm::ivec3 coordinates) -> void
{
    auto tex_coords = blocks_texture_atlas[get_block_texture_index(block, facing)];
    auto& face = get_face(facing);

    for (usize i = 0; i < vertices_per_quad; i++)
    {
        zth::StandardVertex vertex = {
            .local_position = face.vertices[i] + glm::vec3{ coordinates },
            .normal = face.normal,
            .tex_coords = tex_coords[i],
        };

        mesh.vertices.push_back(vertex);
    }

    mesh.faces++;
}

auto append_block_vertices(ChunkMesh& mesh, Block block, BlockFacing facing, glm::ivec3 coordinates) -> void
{
    if (block == Block::Air)
        return;

    if (facing & Facing_Backward)
        append_single_face_vertices(mesh, block, Facing_Backward, coordinates);

    if (facing & Facing_Forward)
        append_single_face_vertices(mesh, block, Facing_Forward, coordinates);

    if (facing & Facing_Left)
        append_single_face_vertices(mesh, block, Facing_Left, coordinates);

    if (facing & Facing_Right)
        append_single_face_vertices(mesh, block, Facing_Right, coordinates);

    if (facing & Facing_Down)
        append_single_face_vertices(mesh, block, Facing_Down, coordinates);

    if (facing & Facing_Up)
        append_single_face_vertices(mesh, block, Facing_Up, coordinates);
}

} // namespace

Chunk::Chunk(zth::UniquePtr<ChunkData>&& blocks) : _blocks(std::move(blocks)) {}

auto Chunk::generate_mesh() const -> ChunkMesh
{
    ChunkMesh result{};
    // @todo: Check if reserving some space for the vertices here would be good.
    // result.vertices.reserve(static_cast<usize>(block_count) * 6);

    constexpr auto xs = std::views::iota(0, chunk_size.x);
    constexpr auto ys = std::views::iota(0, chunk_size.y);
    constexpr auto zs = std::views::iota(0, chunk_size.z);
    constexpr auto coords = std::views::cartesian_product(xs, ys, zs);

    for (const auto [x, y, z] : coords)
    {
        auto block_coords = glm::ivec3{ x, y, z };
        auto block = operator[](block_coords);
        auto facing = visible_faces_for_block(block_coords);
        append_block_vertices(result, block, facing, block_coords);
    }

    return result;
}

auto Chunk::upload_mesh(const ChunkMesh& mesh) -> void
{
    _mesh = std::make_shared<zth::QuadMesh>(mesh.vertices, zth::StandardVertex::layout);
}

auto Chunk::at(glm::ivec3 coordinates) const -> zth::Optional<Block>
{
    if (!valid_coordinates(coordinates))
        return zth::nil;

    return operator[](coordinates);
}

auto Chunk::operator[](glm::ivec3 coordinates) -> Block&
{
    ZTH_ASSERT(valid_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    std::mdspan view{ _blocks->data(), chunk_size.x, chunk_size.y, chunk_size.z };
    return view[x, y, z];
}

auto Chunk::operator[](glm::ivec3 coordinates) const -> const Block&
{
    ZTH_ASSERT(valid_coordinates(coordinates));
    auto [x, y, z] = coordinates;
    std::mdspan view{ _blocks->data(), chunk_size.x, chunk_size.y, chunk_size.z };
    return view[x, y, z];
}

auto Chunk::to_world_x(i32 x) -> i32
{
    return x * chunk_size.x;
}

auto Chunk::to_world_z(i32 z) -> i32
{
    return z * chunk_size.z;
}

auto Chunk::visible_faces_for_block(glm::ivec3 coordinates) const -> BlockFacing
{
    auto [x, y, z] = coordinates;
    auto facing = Facing_None;

    auto backward = at(glm::vec3{ x, y, z + 1 });
    auto forward = at(glm::vec3{ x, y, z - 1 });
    auto left = at(glm::vec3{ x - 1, y, z });
    auto right = at(glm::vec3{ x + 1, y, z });
    auto down = at(glm::vec3{ x, y - 1, z });
    auto up = at(glm::vec3{ x, y + 1, z });

    if (!backward || backward == Block::Air)
        facing |= Facing_Backward;

    if (!forward || forward == Block::Air)
        facing |= Facing_Forward;

    if (!left || left == Block::Air)
        facing |= Facing_Left;

    if (!right || right == Block::Air)
        facing |= Facing_Right;

    if (!down || down == Block::Air)
        facing |= Facing_Down;

    if (!up || up == Block::Air)
        facing |= Facing_Up;

    return facing;
}

auto Chunk::valid_coordinates(glm::ivec3 coordinates) -> bool
{
    auto [x, y, z] = coordinates;

    auto valid_x = x >= 0 && x < chunk_size.x;
    auto valid_y = y >= 0 && y < chunk_size.y;
    auto valid_z = z >= 0 && z < chunk_size.z;

    if (valid_x && valid_y && valid_z)
        return true;

    return false;
}

auto ChunkGenerator::generate(ChunkPosition position) -> zth::UniquePtr<ChunkData>
{
    // zth::make_unique_for_overwrite doesn't initialize the data, and we're going to overwrite all of it anyway.
    zth::UniquePtr<ChunkData> blocks = zth::make_unique_for_overwrite<ChunkData>();
    std::mdspan view{ blocks->data(), chunk_size.x, chunk_size.y, chunk_size.z };

    auto chunk_start_x = Chunk::to_world_x(position.x);
    auto chunk_start_z = Chunk::to_world_z(position.z);

    for (i32 x = 0; x < chunk_size.x; x++)
    {
        for (i32 z = 0; z < chunk_size.z; z++)
        {
            auto height = get_height(chunk_start_x + x, chunk_start_z + z);

            for (i32 y = 0; y < chunk_size.y; y++)
            {
                if (y > height)
                    view[x, y, z] = Block::Air;
                else if (y == height)
                    view[x, y, z] = Block::Grass;
                else if (y > height - 4)
                    view[x, y, z] = Block::Dirt;
                else
                    view[x, y, z] = Block::Stone;
            }
        }
    }

    return blocks;
}

auto ChunkGenerator::get_height(i32 world_x, i32 world_z) -> i32
{
    glm::vec2 position{ world_x, world_z };
    position *= scale;

    auto noise = zth::Random::perlin_noise(position) * 0.5f + 0.5f;
    auto height = std::lerp(min_height, max_height, noise);

    return static_cast<i32>(height * static_cast<float>(chunk_size.y));
}
