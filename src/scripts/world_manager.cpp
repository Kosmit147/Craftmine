#include "scripts/world_manager.hpp"

#include "assets.hpp"
#include "world/generator.hpp"

namespace scripts {

using namespace zth::hashed_string_literals;
using namespace std::chrono_literals;

WorldManager::WorldManager(zth::ConstEntityHandle player) : player{ player } {}

auto WorldManager::debug_edit() -> void
{
    zth::debug::slide_int("Distance", distance, 0, 100);

    zth::debug::input_int("Max load chunk tasks", max_load_chunk_tasks);
    zth::debug::input_int("Max update chunk tasks", max_update_chunk_tasks);

    zth::debug::input_int("Max chunks loaded each frame", max_chunks_loaded_each_frame);
    zth::debug::input_int("Max chunks updated each frame", max_chunks_updated_each_frame);

    zth::debug::text("Unhandled unload chunk requests: {}", _unload_chunk_requests.size());
    zth::debug::text("Unhandled load chunk requests: {}", _load_chunk_requests.size());
    zth::debug::text("Unhandled update chunk requests: {}", _update_chunk_requests.size());

    if (zth::debug::button("Clear world"))
        clear_world();
}

auto WorldManager::on_update([[maybe_unused]] zth::EntityHandle actor) -> void
{
    auto player_chunk = get_player_chunk();

    request_to_load_chunks_around_player(player_chunk);
    request_to_unload_chunks_too_far_away_from_player(player_chunk);

    // Process unload chunk requests.
    while (!_unload_chunk_requests.empty())
    {
        auto chunk_position = _unload_chunk_requests.front();
        unload_chunk(chunk_position);
        _unload_chunk_requests.pop_front();
    }

    // Process load chunk requests.
    while (!_load_chunk_requests.empty() && _load_chunk_tasks.size() < max_load_chunk_tasks)
    {
        auto chunk_position = _load_chunk_requests.front();

        if (get_distance(player_chunk, chunk_position) <= distance && !_chunk_map.contains(chunk_position))
        {
            auto [_, success] = _chunk_map.emplace(chunk_position, create_new_chunk_entity(chunk_position));
            ZTH_ASSERT(success);
            launch_load_chunk_task(chunk_position);
        }

        _load_chunk_requests.pop_front();
    }

    // Get results from load chunk tasks.
    for (usize i = 0, chunks_loaded_already = 0;
         chunks_loaded_already < max_chunks_loaded_each_frame && i < _load_chunk_tasks.size(); i++)
    {
        auto& task = _load_chunk_tasks[i];

        if (task.wait_for(0s) == std::future_status::ready)
        {
            auto [chunk_position, chunk_data] = task.get();

            if (auto chunk_entity = get_chunk(chunk_position))
            {
                update_chunk_entity_with_data(*chunk_entity, std::move(chunk_data));
                update_neighbor_arrays_on_chunk_loaded(*chunk_entity);

                request_to_update_chunk(chunk_position);
                request_to_update_neighbors(chunk_position);
            }

            _load_chunk_tasks.erase(std::next(_load_chunk_tasks.begin(), static_cast<zth::isize>(i)));
            i = std::min(0uLL, i - 1); // Go back in the loop because we erased one element.
            chunks_loaded_already++;
        }
    }

    // Process update chunk requests.
    while (!_update_chunk_requests.empty() && _update_chunk_tasks.size() < max_update_chunk_tasks)
    {
        auto chunk_position = _update_chunk_requests.front();

        if (auto chunk_entity = get_chunk(chunk_position))
            launch_update_chunk_task(*chunk_entity);

        _update_chunk_requests.pop_front();
    }

    // Get results from update chunk tasks.
    for (usize i = 0, chunks_updated_already = 0;
         chunks_updated_already < max_chunks_updated_each_frame && i < _update_chunk_tasks.size(); i++)
    {
        auto& task = _update_chunk_tasks[i];

        if (task.wait_for(0s) == std::future_status::ready)
        {
            auto [chunk_entity, chunk_mesh] = task.get();

            if (chunk_entity.valid())
                update_chunk_entity(chunk_entity, chunk_mesh);

            _update_chunk_tasks.erase(std::next(_update_chunk_tasks.begin(), static_cast<zth::isize>(i)));
            i = std::min(0uLL, i - 1); // Go back in the loop because we erased one element.
            chunks_updated_already++;
        }
    }
}

auto WorldManager::on_attach([[maybe_unused]] zth::EntityHandle actor) -> void
{
    _scene = &zth::SceneManager::scene();

    _blocks_texture =
        zth::AssetManager::emplace<zth::gl::Texture2D>(
            "blocks_texture"_hs,
            zth::gl::Texture2D::from_file_data(
                blocks_texture_data, zth::gl::TextureParams{ .mag_filter = zth::gl::TextureMagFilter::nearest }))
            ->get();

    _chunk_material =
        zth::AssetManager::emplace<zth::Material>("chunk_material"_hs, zth::Material{ .diffuse_map = _blocks_texture })
            ->get();
}

auto WorldManager::on_detach([[maybe_unused]] zth::EntityHandle actor) -> void
{
    clear_world();

    zth::AssetManager::remove<zth::gl::Texture2D>("blocks_texture"_hs);
    zth::AssetManager::remove<zth::Material>("chunk_material"_hs);

    _blocks_texture.reset();
    _chunk_material.reset();
}

auto WorldManager::get_chunk(glm::ivec2 chunk_position) -> Optional<zth::EntityHandle>
{
    if (auto kv = _chunk_map.find(chunk_position); kv != _chunk_map.end())
    {
        auto& [_, entity] = *kv;
        return entity;
    }

    return nil;
}

auto WorldManager::get_chunk(glm::ivec2 chunk_position) const -> Optional<zth::ConstEntityHandle>
{
    if (auto kv = _chunk_map.find(chunk_position); kv != _chunk_map.end())
    {
        auto& [_, entity] = *kv;
        return entity;
    }

    return nil;
}

auto WorldManager::request_to_load_chunks_around_player(glm::ivec2 player_chunk) -> void
{
    if (_load_chunk_requests.size() > 5000)
        return;

    // Load chunks in a spiral pattern starting with the chunks which are the furthest away from the player. Since we
    // want to prioritize the chunks closest to the player, we want to push them onto the front of the queue last.

    for (i32 i = distance; i >= 0; i--)
    {
        // Top Row.
        for (i32 x = -i; x < i; x++)
            request_to_load_chunk_with_priority(player_chunk + glm::ivec2{ x, i });

        // Right Column.
        for (i32 z = i; z > -i; z--)
            request_to_load_chunk_with_priority(player_chunk + glm::ivec2{ i, z });

        // Bottom Row.
        for (i32 x = i; x > -i; x--)
            request_to_load_chunk_with_priority(player_chunk + glm::ivec2{ x, -i });

        // Left Column.
        for (i32 z = -i; z < i; z++)
            request_to_load_chunk_with_priority(player_chunk + glm::ivec2{ -i, z });
    }

    // Loop above doesn't handle the player chunk.
    if (distance >= 0)
        request_to_load_chunk_with_priority(player_chunk);
}

auto WorldManager::request_to_unload_chunks_too_far_away_from_player(glm::ivec2 player_chunk) -> void
{
    for (const auto& chunk_position : _chunk_map | std::views::keys)
    {
        if (get_distance(player_chunk, chunk_position) > distance)
            request_to_unload_chunk(chunk_position);
    }
}

auto WorldManager::request_to_load_chunk(glm::ivec2 chunk_position) -> void
{
    _load_chunk_requests.push_back(chunk_position);
}

auto WorldManager::request_to_load_chunk_with_priority(glm::ivec2 chunk_position) -> void
{
    _load_chunk_requests.push_front(chunk_position);
}

auto WorldManager::launch_load_chunk_task(glm::ivec2 chunk_position) -> void
{
    _load_chunk_tasks.push_back(
        std::async(std::launch::async, [chunk_position] { return load_chunk(chunk_position); }));
}

auto WorldManager::load_chunk(glm::ivec2 chunk_position) -> std::pair<glm::ivec2, std::shared_ptr<ChunkData>>
{
    // @multithreaded

    return { chunk_position, WorldGenerator::generate(chunk_position) };
}

auto WorldManager::create_new_chunk_entity(glm::ivec2 chunk_position) -> zth::EntityHandle
{
    auto [x, z] = chunk_position;
    auto entity = _scene->create_entity(zth::format("Chunk (x: {}, z: {})", x, z));
    entity.transform().set_translation({ chunk_x_to_world_x(x), 0.0f, chunk_z_to_world_z(z) });
    entity.emplace<ChunkComponent>(nullptr, get_neighbors(chunk_position), chunk_position);
    entity.emplace<zth::MaterialComponent>(_chunk_material);
    return entity;
}

auto WorldManager::update_chunk_entity_with_data(zth::EntityHandle chunk_entity,
                                                 std::shared_ptr<ChunkData>&& chunk_data) -> void
{
    ZTH_ASSERT(chunk_entity.valid());
    chunk_entity.patch<ChunkComponent>([&chunk_data](auto& component) { component.data = std::move(chunk_data); });
}

auto WorldManager::update_neighbor_arrays_on_chunk_loaded(zth::EntityHandle chunk_entity) -> void
{
    // Update the neighbor arrays of neighboring chunks to hold a reference to the data of the newly loaded chunk.

    ZTH_ASSERT(chunk_entity.valid());
    const auto& [chunk_data, _, chunk_position] = chunk_entity.get<const ChunkComponent>();

    for (usize i = 0; i < neighbor_count; i++)
    {
        if (auto neighbor = get_chunk(chunk_position + neighbor_offsets[i]))
        {
            auto& neighbors_array = neighbor->get<ChunkComponent>().neighbors;
            neighbors_array[opposite_neighbor_offset_idx[i]] = chunk_data;
        }
    }
}

auto WorldManager::request_to_update_chunk(glm::ivec2 chunk_position) -> void
{
    _update_chunk_requests.push_back(chunk_position);
}

auto WorldManager::request_to_update_chunk_with_priority(glm::ivec2 chunk_position) -> void
{
    _update_chunk_requests.push_front(chunk_position);
}

auto WorldManager::request_to_update_neighbors(glm::ivec2 chunk_position) -> void
{
    for (auto coord : neighbor_offsets)
        request_to_update_chunk(chunk_position + coord);
}

auto WorldManager::request_to_update_neighbors_with_priority(glm::ivec2 chunk_position) -> void
{
    for (auto coord : neighbor_offsets)
        request_to_update_chunk_with_priority(chunk_position + coord);
}

auto WorldManager::launch_update_chunk_task(zth::EntityHandle chunk_entity) -> void
{
    auto [chunk_data, chunk_neighbors, chunk_position] = chunk_entity.get<const ChunkComponent>();

    if (!chunk_data)
        return;

    _update_chunk_tasks.push_back(std::async(
        std::launch::async, [chunk_entity, data = std::move(chunk_data), neighbors = std::move(chunk_neighbors)] {
            return update_chunk(chunk_entity, *data, neighbors);
        }));
}

auto WorldManager::update_chunk(zth::EntityHandle chunk_entity, const ChunkData& chunk_data,
                                const NeighborsArray& neighbors)
    -> std::pair<zth::EntityHandle, zth::Vector<zth::StandardVertex>>
{
    // @multithreaded

    return { chunk_entity, chunk_data.generate_mesh(neighbors) };
}

auto WorldManager::update_chunk_entity(zth::EntityHandle chunk_entity,
                                       const zth::Vector<zth::StandardVertex>& chunk_mesh) -> void
{
    ZTH_ASSERT(chunk_entity.valid());
    chunk_entity.emplace_or_replace<zth::MeshRendererComponent>(std::make_shared<zth::QuadMesh<>>(chunk_mesh));
}

auto WorldManager::request_to_unload_chunk(glm::ivec2 chunk_position) -> void
{
    _unload_chunk_requests.push_back(chunk_position);
}

auto WorldManager::unload_chunk(glm::ivec2 chunk_position) -> void
{
    if (auto chunk_entity = get_chunk(chunk_position))
    {
        chunk_entity->destroy();
        _chunk_map.erase(chunk_position);
    }
}

auto WorldManager::get_player_chunk() const -> glm::ivec2
{
    if (!player)
        return { 0, 0 };

    auto player_position = glm::ivec3{ player.transform().translation() };
    return { world_x_to_chunk_x(player_position.x), world_z_to_chunk_z(player_position.z) };
}

auto WorldManager::get_distance(glm::ivec2 chunk_a, glm::ivec2 chunk_b) -> i32
{
    auto [x, z] = chunk_b - chunk_a;
    return std::max(std::abs(x), std::abs(z));
}

auto WorldManager::get_neighbors(glm::ivec2 chunk_position) const -> NeighborsArray
{
    NeighborsArray neighbors{};

    for (usize i = 0; i < neighbors.size(); i++)
    {
        auto neighbor = get_chunk(chunk_position + neighbor_offsets[i]);
        neighbors[i] = neighbor ? neighbor->get<const ChunkComponent>().data : nullptr;
    }

    return neighbors;
}

auto WorldManager::clear_world() -> void
{
    for (auto& chunk_entity : _chunk_map | std::views::values)
        chunk_entity.destroy();

    _chunk_map.clear();

    _unload_chunk_requests.clear();

    _load_chunk_requests.clear();
    _load_chunk_tasks.clear();

    _update_chunk_requests.clear();
    _update_chunk_tasks.clear();
}

} // namespace scripts
