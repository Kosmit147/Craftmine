#include "assets.hpp"

#include <battery/embed.hpp>

const std::span<const std::byte> blocks_texture_data = b::embed<"assets/textures/blocks.png">().data();
const std::span<const std::byte> crosshair_texture_data = b::embed<"assets/textures/Crosshair.png">().data();
const std::span<const std::byte> exit_button_texture_data = b::embed<"assets/textures/ExitButton.png">().data();
const std::span<const std::byte> exit_button_hover_texture_data = b::embed<"assets/textures/ExitButtonHover.png">().data();
const std::span<const std::byte> logo_texture_data = b::embed<"assets/textures/Logo.png">().data();
// const std::span<const std::byte> menu_background_texture_data = b::embed<"assets/textures/MenuBackground.png">().data();
const std::span<const std::byte> start_button_texture_data = b::embed<"assets/textures/StartButton.png">().data();
const std::span<const std::byte> start_button_hover_texture_data = b::embed<"assets/textures/StartButtonHover.png">().data();
