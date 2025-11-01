#include  "core/renderer/renderer.h"


std::shared_ptr<Framebuffer> Renderer::get_shadow_map_fbo() const {
    return shadow_map_fbo;
}

std::shared_ptr<Framebuffer> Renderer::get_main_fbo() const {
    return main_fbo;
}
