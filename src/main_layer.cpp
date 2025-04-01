#include "main_layer.hpp"

auto MainLayer::on_event(const zth::Event& event) -> void
{
    if (event.type() == zth::EventType::KeyPressed)
    {
        auto [key] = event.key_pressed_event();

        if (key == zth::Key::Escape)
            zth::Window::close();
        else if (key == zth::Key::LeftControl)
            zth::Window::set_cursor_enabled(!zth::Window::cursor_enabled());
    }
}
