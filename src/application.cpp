#include "application.hpp"

#include "main_layer.hpp"
#include "main_scene.hpp"

ZTH_IMPLEMENT_APP(Application)

const zth::ApplicationSpec application_spec = {
    .window_spec = {
        .size = { 800, 600 },
        .title = "Craftmine",
        .gl_version = { 4, 6 },
        .gl_profile = zth::gl::Profile::Core,
        .fullscreen = false,
        .vsync = true,
        .frame_rate_limit = 60,
        .resizable = true,
        .maximized = true,
        .cursor_enabled = false,
        .transparent_framebuffer = false,
        .forced_aspect_ratio = zth::nil,
    },
    .logger_spec = {
        .client_logger_label = "CRAFTMINE",
    }
};

Application::Application() : zth::Application(application_spec)
{
    push_layer(std::make_unique<MainLayer>());
    zth::SceneManager::queue_scene(std::make_unique<MainScene>());
}
