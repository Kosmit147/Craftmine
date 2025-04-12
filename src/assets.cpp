#include "assets.hpp"
#include <battery/embed.hpp>

const std::span<const std::uint8_t> blocks_texture_data = b::embed<"assets/textures/blocks.png">().data();
