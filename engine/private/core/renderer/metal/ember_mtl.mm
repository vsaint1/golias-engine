#import "core/renderer/metal/ember_mtl.h"

#import <SDL3/SDL.h>
#import <SDL3/SDL_metal.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <stb_image.h>



MetalRenderer::MetalRenderer() = default;
MetalRenderer::~MetalRenderer() = default;

void MetalRenderer::initialize() {



}

void MetalRenderer::clear(glm::vec4 color) {

}

std::shared_ptr<Texture> MetalRenderer::load_texture(const std::string& path) {
    return {};
}

void MetalRenderer::draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) {

}

void MetalRenderer::draw_texture(const Texture* texture, const Rect2& dest_rect, float rotation, const glm::vec4& color,
                                 const Rect2& src_rect, int z_index, bool flip_h, bool flip_v, const UberShader& uber_shader) {

}

void MetalRenderer::draw_circle(float cx, float cy, float rotation, float radius, const glm::vec4& color, bool filled, int z_index, int segments) {

}

void MetalRenderer::flush() {

}

void MetalRenderer::present() {
    // This is typically called after flush(), but flush() already handles presentation
}

void MetalRenderer::resize_viewport(int w, int h) {

}

void MetalRenderer::destroy() {
    if (pipeline_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_;
        (void)p;
        pipeline_ = nullptr;
    }
    if (sampler_) {
        id<MTLSamplerState> s = (__bridge_transfer id<MTLSamplerState>)sampler_;
        (void)s;
        sampler_ = nullptr;
    }
    if (queue_) {
        id<MTLCommandQueue> q = (__bridge_transfer id<MTLCommandQueue>)queue_;
        (void)q;
        queue_ = nullptr;
    }

    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if (it->second && it->second->mtlTexture) {
            id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)(it->second->mtlTexture);
            (void)tex;
            it->second->mtlTexture = nullptr;
        }
    }

    _textures.clear();
    _texture_sizes.clear();

    if (metal_view_) {
        SDL_Metal_DestroyView(metal_view_);
        metal_view_ = nullptr;
    }

    device_ = nullptr;
    metal_layer_ = nullptr;
}

void MetalRenderer::unload_texture(Uint32 tex_id) {
    if (tex_id == 0) return;
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if (it->second && it->second->id == tex_id) {
            if (it->second->mtlTexture) {
                id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)it->second->mtlTexture;
                (void)tex;
                it->second->mtlTexture = nullptr;
            }
            _texture_sizes.erase(tex_id);
            _textures.erase(it);
            break;
        }
    }
}

std::shared_ptr<Texture> MetalRenderer::get_texture(const std::string& path) {
    auto [it, inserted] = _textures.try_emplace(path, nullptr);

    if (inserted) {
        it->second = load_texture(path);
    }

    return it->second;
}

void MetalRenderer::set_context(const void* ctx) {}

void* MetalRenderer::get_context() { return device_; }

// --- stubs ---
bool MetalRenderer::load_font(const std::string&, const std::string&, int) { return false; }
void MetalRenderer::unload_font(const Font&) {}
void MetalRenderer::draw_text(const std::string&, float, float, float, float, const glm::vec4&,
                              const std::string&, int, const UberShader&, int) {}
void MetalRenderer::draw_line(float, float, float, float, float, float, const glm::vec4&, int) {}
void MetalRenderer::draw_triangle(float, float, float, float, float, float,
                                  float, const glm::vec4&, bool, int) {}
void MetalRenderer::draw_polygon(const std::vector<glm::vec2>&, float, const glm::vec4&, bool, int) {}
void MetalRenderer::set_default_font(const std::string&) {}
void MetalRenderer::render_fbo() {}
void MetalRenderer::set_effect_uniforms(const UberShader&, const glm::vec2&) {}
glm::vec2 MetalRenderer::get_texture_size(Uint32) const { return {0,0}; }