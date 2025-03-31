#include "application.hpp"

#include "scene.hpp"

// @todo: Fix ::
ZTH_IMPLEMENT_APP(::Application)

const zth::ApplicationSpec app_spec = {
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
};

Application::Application() : zth::Application(app_spec)
{
    zth::SceneManager::load_scene(std::make_unique<Scene>());
}
