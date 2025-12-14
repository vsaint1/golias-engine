#include "servers/rendering/rendering_canvas.h"

#include "stdafx.h"


RenderingCanvas::RenderingCanvas(RenderingDevice* device)
    : rd(device), shader(INVALID_RID), pipeline(INVALID_RID), vertex_buffer(INVALID_RID), index_buffer(INVALID_RID),
      white_texture(INVALID_RID), default_sampler(INVALID_RID), current_blend_mode(BlendMode::ALPHA), scale_mode(ScaleMode::KEEP),
      line_width(1.0f), window_width(800), window_height(600), viewport_width(800), viewport_height(600), is_drawing(false) {
}

RenderingCanvas::~RenderingCanvas() {
    shutdown();
}

bool RenderingCanvas::initialize(int width, int height) {

    window_width    = width;
    window_height   = height;
    viewport_width  = width;
    viewport_height = height;

    text_storage.emoji_font = Font(TTF_OpenFont((ASSETS_PATH + "fonts/Twemoji.ttf").c_str(), 24));

    if (!text_storage.emoji_font.get_native_handle()) {
        spdlog::warn("Failed to load emoji font (Twemoji.ttf), emoji support disabled: {}", SDL_GetError());
    } else {
        spdlog::info("Loaded emoji font (Twemoji.ttf) for emoji support.");
    }

    text_storage.default_font = Font(TTF_OpenFont((ASSETS_PATH + "fonts/Default.ttf").c_str(), 24));

    if (!text_storage.default_font.get_native_handle()) {
        spdlog::error("Failed to load Default font (Default.ttf): {}", SDL_GetError());
        return false;
    }

    if (text_storage.emoji_font.get_native_handle()) {
        if (!TTF_AddFallbackFont(text_storage.default_font.get_native_handle(), text_storage.emoji_font.get_native_handle())) {
            spdlog::warn("Failed to add emoji fallback font: {}", SDL_GetError());
        }
    }

    // TODO: we need to handle shader compilations (glslang, spirv-cross and dxcompiler)
    shader = rd->shader_create_from_source(shaders::get_default_vertex_2d(), shaders::get_default_fragment_2d());


    if (shader == INVALID_RID) {
        spdlog::error("Failed to create default shader for RenderingCanvas.");
        return false;
    }

    uint8_t white_pixel[] = {255, 255, 255, 255};
    white_texture         = load_texture_from_memory(white_pixel, 1, 1, 4);

    SamplerState sampler_state;
    sampler_state.min_filter = TextureFilter::LINEAR;
    sampler_state.mag_filter = TextureFilter::LINEAR;
    default_sampler          = rd->sampler_create(sampler_state);

    const size_t max_vertices = 10000;
    const size_t max_indices  = 15000;

    vertex_buffer = rd->buffer_create(max_vertices * sizeof(Vertex), (uint32_t) BufferUsage::BUFFER_USAGE_VERTEX, nullptr);

    index_buffer = rd->buffer_create(max_indices * sizeof(uint16_t), (uint32_t) BufferUsage::BUFFER_USAGE_INDEX, nullptr);

    PipelineState pipeline_state;
    pipeline_state.shader                          = shader;
    pipeline_state.topology                        = PrimitiveTopology::TRIANGLES;
    pipeline_state.vertex_format.stride            = sizeof(Vertex);
    pipeline_state.vertex_format.attributes        = {{0, DataFormat::R32G32B32_SFLOAT, offsetof(Vertex, position)},
                                                      {1, DataFormat::R32G32B32A32_SFLOAT, offsetof(Vertex, color)},
                                                      {2, DataFormat::R32G32_SFLOAT, offsetof(Vertex, texcoord)}};
    pipeline_state.rasterization.cull_mode         = CullMode::NONE;
    pipeline_state.depth_stencil.depth_test_enable = false;

    BlendState blend;
    blend.enable    = true;
    blend.src_color = BlendFactor::SRC_ALPHA;
    blend.dst_color = BlendFactor::ONE_MINUS_SRC_ALPHA;
    blend.src_alpha = BlendFactor::ONE;
    blend.dst_alpha = BlendFactor::ONE_MINUS_SRC_ALPHA;
    pipeline_state.blend_states.push_back(blend);

    pipeline = rd->pipeline_create(pipeline_state);

    if (pipeline == INVALID_RID) {
        return false;
    }

    reset_camera();
    return true;
}

void RenderingCanvas::shutdown() {
    if (pipeline != INVALID_RID) {
        rd->pipeline_destroy(pipeline);
    }

    if (vertex_buffer != INVALID_RID) {
        rd->buffer_destroy(vertex_buffer);
    }

    if (index_buffer != INVALID_RID) {
        rd->buffer_destroy(index_buffer);
    }

    if (white_texture != INVALID_RID) {
        rd->texture_destroy(white_texture);
    }

    if (default_sampler != INVALID_RID) {
        rd->sampler_destroy(default_sampler);
    }

    if (shader != INVALID_RID) {
        rd->shader_destroy(shader);
    }


    for (auto& pair : custom_shader_pipelines) {
        rd->pipeline_destroy(pair.second);
    }

    for (auto& pair : text_storage.loaded_fonts) {
        pair.second.destroy();
    }

    text_storage.loaded_fonts.clear();

    text_storage.emoji_font.destroy();

    text_storage.default_font.destroy();

    custom_shader_pipelines.clear();
}

void RenderingCanvas::begin(const Color& color) {
    is_drawing = true;
    vertices.clear();
    indices.clear();
    draw_commands.clear();

    clear_color = color;
}

void RenderingCanvas::end() {
    int vp_x, vp_y, vp_w, vp_h;
    calculate_viewport(viewport_width, viewport_height, vp_x, vp_y, vp_w, vp_h);

    Viewport viewport{(float) vp_x, (float) vp_y, (float) vp_w, (float) vp_h, 0.0f, 1.0f};
    Scissor scissor{0, 0, (uint32_t) viewport_width, (uint32_t) viewport_height};

    if (!vertices.empty() && !indices.empty()) {
        rd->buffer_update(vertex_buffer, 0, vertices.size() * sizeof(Vertex), vertices.data());
        rd->buffer_update(index_buffer, 0, indices.size() * sizeof(uint16_t), indices.data());
    }

    rd->begin_frame();
    // Set clear color BEFORE render_pass_begin so SDL GPU can use it
    rd->clear_color(clear_color.to_vec4());
    rd->render_pass_begin(INVALID_RID, viewport, scissor);

    flush();

    rd->render_pass_end();
    rd->end_frame();

    /// Each texture loaded for text rendering is temporary and should be unloaded after each frame
    for (RID tex : text_storage.textures) {
        unload_texture(tex);
    }

    text_storage.textures.clear();

    is_drawing = false;
}

void RenderingCanvas::flush() {
    if (vertices.empty() || indices.empty()) {
        return;
    }

    // Buffer updates already done in end() before render pass started
    // flush();

    rd->bind_pipeline(pipeline);
    rd->bind_vertex_buffers({vertex_buffer});
    rd->bind_index_buffer(index_buffer, IndexType::UINT16);

    glm::mat4 vp = projection * view;
    rd->push_constant("VIEW_PROJECTION_MATRIX", glm::value_ptr(vp), sizeof(glm::mat4));

    for (const auto& cmd : draw_commands) {
        RID tex = cmd.use_texture ? cmd.texture : white_texture;


        RID sampler     = default_sampler;
        auto sampler_it = texture_samplers.find(tex);

        if (sampler_it != texture_samplers.end()) {
            sampler = sampler_it->second;
        }

        rd->bind_texture(0, tex, sampler);


        rd->draw_indexed(cmd.index_count, 1, cmd.index_start, 0, 0);
    }

    vertices.clear();
    indices.clear();
    draw_commands.clear();
}

void RenderingCanvas::draw_rect(float x, float y, float width, float height, const Color& color, float rotation) {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + width / 2, y + height / 2, 0));

    if (rotation != 0.0f) {
        transform = glm::rotate(transform, rotation, glm::vec3(0, 0, 1));
    }

    transform = glm::scale(transform, glm::vec3(width, height, 1));

    add_quad(get_current_transform() * transform, color, white_texture, false);
}

void RenderingCanvas::draw_rect_outlined(float x, float y, float width, float height, const Color& color, float thickness) {

    draw_rect(x, y, width, thickness, color); // TOP

    draw_rect(x, y + height - thickness, width, thickness, color); // BOTTOM

    draw_rect(x, y + thickness, thickness, height - 2 * thickness, color); // LEFT

    draw_rect(x + width - thickness, y + thickness, thickness, height - 2 * thickness, color); // RIGHT
}

void RenderingCanvas::draw_circle(float x, float y, float radius, const Color& color, int segments) {
    Vector<glm::vec2> points;

    for (int i = 0; i < segments; i++) {
        float angle = (float) i / segments * 2.0f * 3.14159265359f;
        points.push_back(glm::vec2(x + std::cos(angle) * radius, y + std::sin(angle) * radius));
    }

    draw_polygon(points, color);
}

void RenderingCanvas::draw_circle_outlined(float x, float y, float radius, const Color& color, float thickness, int segments) {
    if (segments < 3) {
        segments = 32;
    }

    const float angle_step = 2.0f * glm::pi<float>() / segments;

    for (int i = 0; i < segments; i++) {
        float angle1 = i * angle_step;
        float angle2 = (i + 1) * angle_step;

        float x1 = x + glm::cos(angle1) * radius;
        float y1 = y + glm::sin(angle1) * radius;
        float x2 = x + glm::cos(angle2) * radius;
        float y2 = y + glm::sin(angle2) * radius;

        draw_line(x1, y1, x2, y2, color, thickness);
    }
}

void RenderingCanvas::draw_arc(float x, float y, float radius, float start_angle, float end_angle, const Color& color, int segments) {
    if (segments < 3) {
        segments = 32;
    }

    while (start_angle < 0.0f) {
        start_angle += 2.0f * glm::pi<float>();
    }
    while (end_angle < 0.0f) {
        end_angle += 2.0f * glm::pi<float>();
    }
    while (start_angle >= 2.0f * glm::pi<float>()) {
        start_angle -= 2.0f * glm::pi<float>();
    }
    while (end_angle >= 2.0f * glm::pi<float>()) {
        end_angle -= 2.0f * glm::pi<float>();
    }

    float angle_span = end_angle - start_angle;
    if (angle_span < 0.0f) {
        angle_span += 2.0f * glm::pi<float>();
    }

    int arc_segments = SDL_max(3, static_cast<int>(segments * (angle_span / (2.0f * glm::pi<float>()))));

    Vector<glm::vec2> points;

    points.push_back(glm::vec2(x, y));

    for (int i = 0; i <= arc_segments; i++) {
        float t     = static_cast<float>(i) / arc_segments;
        float angle = start_angle + angle_span * t;
        points.push_back(glm::vec2(x + glm::cos(angle) * radius, y + glm::sin(angle) * radius));
    }

    draw_polygon(points, color);
}

void RenderingCanvas::get_texture_size(RID texture, uint32_t& out_width, uint32_t& out_height) {
    if (texture == INVALID_RID) {
        return;
    }

    rd->get_texture_size(texture, out_width, out_height);
}

Font RenderingCanvas::load_font_from_file(const char* filepath, int size) {

    if (text_storage.loaded_fonts.contains(filepath)) {
        return text_storage.loaded_fonts.at(filepath);
    }

    const Font font = Font(TTF_OpenFont(filepath, (float) size), size);

    if (!font.get_native_handle()) {
        spdlog::error("Failed to load Font from file {}: {}", filepath, SDL_GetError());
        return {};
    }

    if (text_storage.emoji_font.get_native_handle()) {
        if (!TTF_AddFallbackFont(font.get_native_handle(), text_storage.emoji_font.get_native_handle())) {
            spdlog::warn("Failed to add emoji fallback font: {}", SDL_GetError());
        }
    }

    text_storage.loaded_fonts[filepath] = font;

    spdlog::info("Loaded Font from file: {} (size {})", filepath, size);

    return font;
}

void RenderingCanvas::set_blend_mode(BlendMode mode) {
}

void RenderingCanvas::set_line_width(float width) {
}

void RenderingCanvas::present() {
    rd->swap_buffers();
}

RenderingDevice* RenderingCanvas::get_rendering_device() const {
    return rd;
}

void RenderingCanvas::setup_pipeline_for_blend_mode(BlendMode mode) {
}

void RenderingCanvas::draw_line(float x1, float y1, float x2, float y2, const Color& color, float thickness) {
    glm::vec2 dir = glm::normalize(glm::vec2(x2 - x1, y2 - y1));
    glm::vec2 perp(-dir.y, dir.x);

    const float half_thick = thickness * 0.5f;

    glm::vec2 p1 = glm::vec2(x1, y1) + perp * half_thick;
    glm::vec2 p2 = glm::vec2(x1, y1) - perp * half_thick;
    glm::vec2 p3 = glm::vec2(x2, y2) - perp * half_thick;
    glm::vec2 p4 = glm::vec2(x2, y2) + perp * half_thick;

    draw_polygon({p1, p2, p3, p4}, color);
}

void RenderingCanvas::draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, const Color& color) {
    draw_polygon({glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec2(x3, y3)}, color);
}

void RenderingCanvas::draw_polygon(const Vector<glm::vec2>& points, const Color& color) {
    if (points.size() < 3) {
        return;
    }

    uint16_t base_vertex = vertices.size();
    glm::mat4 transform  = get_current_transform();

    for (const auto& p : points) {
        glm::vec4 pos = transform * glm::vec4(p.x, p.y, 0, 1);
        vertices.push_back({glm::vec3(pos), color.to_vec4(), glm::vec2(0, 0)});
    }

    for (size_t i = 1; i < points.size() - 1; i++) {
        indices.push_back(base_vertex);
        indices.push_back(base_vertex + i);
        indices.push_back(base_vertex + i + 1);
    }

    draw_commands.push_back(
        {white_texture, (uint32_t) (indices.size() - (points.size() - 2) * 3), (uint32_t) ((points.size() - 2) * 3), false});
}

void RenderingCanvas::draw_texture(RID texture, float x, float y, float width, float height, const Color& tint, float rotation) {

    if (width == 0 || height == 0) {
        uint32_t tex_w, tex_h;
        rd->get_texture_size(texture, tex_w, tex_h);
        width  = (float) tex_w;
        height = (float) tex_h;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + width / 2, y + height / 2, 0));

    if (rotation != 0.0f) {
        transform = glm::rotate(transform, rotation, glm::vec3(0, 0, 1));
    }

    transform = glm::scale(transform, glm::vec3(width, height, 1));

    add_quad(get_current_transform() * transform, tint, texture, true);
}

void RenderingCanvas::add_quad(const glm::mat4& transform, const Color& color, RID texture, bool use_texture) {
    uint16_t base_vertex = vertices.size();

    glm::vec4 positions[] = {glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec4(0.5f, -0.5f, 0, 1), glm::vec4(0.5f, 0.5f, 0, 1),
                             glm::vec4(-0.5f, 0.5f, 0, 1)};

    glm::vec2 texcoords[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (int i = 0; i < 4; i++) {
        glm::vec4 pos = transform * positions[i];
        vertices.push_back({glm::vec3(pos), color.to_vec4(), texcoords[i]});
    }

    indices.push_back(base_vertex + 0);
    indices.push_back(base_vertex + 1);
    indices.push_back(base_vertex + 2);
    indices.push_back(base_vertex + 2);
    indices.push_back(base_vertex + 3);
    indices.push_back(base_vertex + 0);

    draw_commands.push_back({texture, (uint32_t) (indices.size() - 6), 6, use_texture});
}

void RenderingCanvas::set_camera(const glm::mat4& view_projection) {
    projection = view_projection;
    view       = glm::mat4(1.0f);
}

void RenderingCanvas::reset_camera() {
    projection = glm::ortho(0.0f, (float) window_width, (float) window_height, 0.0f, -1.0f, 1.0f);
    view       = glm::mat4(1.0f);
}

void RenderingCanvas::set_viewport_size(int width, int height) {
    viewport_width  = width;
    viewport_height = height;
}

void RenderingCanvas::set_scale_mode(ScaleMode mode) {
    scale_mode = mode;
}

void RenderingCanvas::calculate_viewport(int window_w, int window_h, int& out_x, int& out_y, int& out_w, int& out_h) {
    switch (scale_mode) {
    case ScaleMode::NONE:
        out_x = (window_w - window_width) / 2;
        out_y = (window_h - window_height) / 2;
        out_w = window_width;
        out_h = window_height;
        break;

    case ScaleMode::KEEP:
        {
            float target_aspect = (float) window_width / (float) window_height;
            float window_aspect = (float) window_w / (float) window_h;

            if (window_aspect > target_aspect) {
                // pillarbox (black bars on sides)
                out_h = window_h;
                out_w = (int) (window_h * target_aspect);
                out_x = (window_w - out_w) / 2;
                out_y = 0;
            } else {
                //  letterbox (black bars on top/bottom)
                out_w = window_w;
                out_h = (int) (window_w / target_aspect);
                out_x = 0;
                out_y = (window_h - out_h) / 2;
            }
        }
        break;

    case ScaleMode::EXPAND:
        out_x = 0;
        out_y = 0;
        out_w = window_w;
        out_h = window_h;
        break;

    default:
        break;
    }
}

// void RenderingCanvas::push_transform(const glm::mat4& transform) {
//     transform_stack.push_back(transform);
// }

// void RenderingCanvas::pop_transform() {
//     if (!transform_stack.empty()) {
//         transform_stack.pop_back();
//     }
// }

glm::mat4 RenderingCanvas::get_current_transform() const {
    glm::mat4 result(1.0f);

    for (const auto& t : transform_stack) {
        result = result * t;
    }

    return result;
}

RID RenderingCanvas::load_texture(const char* filepath) {
    int width, height, channels;
    stbi_uc* data = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);

    if (!data) {
        spdlog::error("Failed to load texture '{}': {}", filepath, stbi_failure_reason());
        return INVALID_RID;
    }

    spdlog::info("Loaded texture '{}' ({}x{})", filepath, width, height);

    TextureFormat format;
    format.type    = TextureType::TEXTURE_TYPE_2D;
    format.format  = DataFormat::R8G8B8A8_UNORM;
    format.width   = width;
    format.height  = height;
    format.mipmaps = 1;

    RID rid = rd->texture_create(format, data);

    stbi_image_free(data);

    return rid;
}

RID RenderingCanvas::load_texture(const char* filepath, const TextureDescription& desc) {
    int width, height, channels;
    stbi_uc* data = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);

    if (!data) {
        spdlog::error("Failed to load texture '{}': {}", filepath, stbi_failure_reason());
        return INVALID_RID;
    }


    TextureFormat format;
    format.type    = TextureType::TEXTURE_TYPE_2D;
    format.format  = DataFormat::R8G8B8A8_UNORM;
    format.width   = width;
    format.height  = height;
    format.mipmaps = desc.generate_mipmaps ? 1 : 0;

    RID texture = rd->texture_create(format, data);

    if (desc.generate_mipmaps && texture != INVALID_RID) {
        rd->texture_generate_mipmaps(texture);
    }

    stbi_image_free(data);

    if (texture != INVALID_RID) {
        SamplerState sampler_state;
        sampler_state.min_filter = desc.min_filter;
        sampler_state.mag_filter = desc.mag_filter;
        sampler_state.wrap_u     = desc.wrap_u;
        sampler_state.wrap_v     = desc.wrap_v;

        RID sampler               = rd->sampler_create(sampler_state);
        texture_samplers[texture] = sampler;
    }


    spdlog::info("Loaded Texture '{}' Size ({}x{}) Description: mipmaps={}, min_filter={}, mag_filter={}, wrap_u={}, wrap_v={}", filepath,
                 width, height, desc.generate_mipmaps, static_cast<int>(desc.min_filter), static_cast<int>(desc.mag_filter),
                 static_cast<int>(desc.wrap_u), static_cast<int>(desc.wrap_v));

    return texture;
}

Texture RenderingCanvas::load_texture_from_file(const char* filepath) {
    RID rid = load_texture(filepath);

    if (rid != INVALID_RID) {
        return rd->get_texture(rid);
    }

    return Texture{};
}

Texture RenderingCanvas::load_texture_from_file(const char* filepath, const TextureDescription& desc) {
    RID rid = load_texture(filepath, desc);

    if (rid != INVALID_RID) {
        return rd->get_texture(rid);
    }

    return Texture{};
}


RID RenderingCanvas::load_texture_from_memory(void* data, int width, int height, int channels) {
    TextureFormat format;
    format.type    = TextureType::TEXTURE_TYPE_2D;
    format.format  = (channels == 4) ? DataFormat::R8G8B8A8_UNORM : DataFormat::R8G8B8_UNORM;
    format.width   = width;
    format.height  = height;
    format.mipmaps = 1;

    return rd->texture_create(format, data);
}

void RenderingCanvas::unload_texture(RID texture) {
    auto it = texture_samplers.find(texture);
    if (it != texture_samplers.end()) {
        rd->sampler_destroy(it->second);
        texture_samplers.erase(it);
    }
    rd->texture_destroy(texture);
}

void RenderingCanvas::draw_text(Font font, float x, float y, const Color& color, const String& text) {
    if (!font.get_native_handle()) {
        spdlog::warn("DrawText called with null font!");
        return;
    }

    if (text.empty()) {
        return;
    }

    SDL_Color text_color;
    text_color.r = (uint8_t) (color.r * 255);
    text_color.g = (uint8_t) (color.g * 255);
    text_color.b = (uint8_t) (color.b * 255);
    text_color.a = (uint8_t) (color.a * 255);

    SDL_Surface* text_surface = TTF_RenderText_Blended(font.get_native_handle(), text.data(), text.length(), text_color);
    if (!text_surface) {
        spdlog::error("Failed to render Text '{}': {}", text, SDL_GetError());
        return;
    }

    SDL_Surface* rgba_surface = SDL_ConvertSurface(text_surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(text_surface);

    if (!rgba_surface) {
        spdlog::error("Failed to convert Text surface to RGBA: {}", SDL_GetError());
        return;
    }

    RID text_texture = load_texture_from_memory(rgba_surface->pixels, rgba_surface->w, rgba_surface->h, 4);

    if (text_texture == INVALID_RID) {
        SDL_DestroySurface(rgba_surface);
        spdlog::error("Failed to create Texture from Text surface");
        return;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + rgba_surface->w / 2.0f, y + rgba_surface->h / 2.0f, 0));
    transform = glm::scale(transform, glm::vec3(rgba_surface->w, rgba_surface->h, 1));


    text_storage.textures.push_back(text_texture);

    add_quad(get_current_transform() * transform, Color::WHITE, text_texture, true);

    SDL_DestroySurface(rgba_surface);
}

void RenderingCanvas::draw_text(float x, float y, const Color& color, const String& text) {
    if (!text_storage.default_font.get_native_handle()) {
        spdlog::warn("Default font not loaded, cannot draw text.");
        return;
    }

    draw_text(text_storage.default_font, x, y, color, text);
}

RID RenderingCanvas::load_shader_from_source(const char* vertex_src, const char* fragment_src) {
    return rd->shader_create_from_source(vertex_src, fragment_src);
}

RID RenderingCanvas::load_shader_from_file(const char* filepath) {
    ShaderSource parsed = load_shader_file(filepath);

    if (!parsed.is_loaded) {
        spdlog::error("Failed to parse Shader file: {}", filepath);
        return INVALID_RID;
    }

    if (parsed.is_compute) {
        spdlog::error("Compute Shaders not Supported for this Rendering: {}", filepath);
        return INVALID_RID;
    }

    spdlog::info("Creating shader from file: {}", filepath);

     shader = rd->shader_create_from_source(parsed.vertex_source.c_str(), parsed.fragment_source.c_str());

    if (shader == INVALID_RID) {
        spdlog::debug("=== VERTEX SHADER ===\n{}", parsed.vertex_source.c_str());
        spdlog::debug("=== FRAGMENT SHADER ===\n{}", parsed.fragment_source.c_str());
        spdlog::error("Failed to compile shader from file: {}", filepath);
    } else {
        spdlog::info("Shader compiled successfully: {} (RID={})", filepath, shader);
    }

    return shader;
}


RID RenderingCanvas::load_shader_from_source(const char* source) {
    ShaderSource parsed = parse_shader(source);

    return rd->shader_create_from_source(parsed.vertex_source, parsed.fragment_source);
}

void RenderingCanvas::destroy_shader(RID shader) {

    auto it = custom_shader_pipelines.find(shader);

    if (it != custom_shader_pipelines.end()) {
        rd->pipeline_destroy(it->second);
        custom_shader_pipelines.erase(it);
    }

    rd->shader_destroy(shader);
}

RID RenderingCanvas::get_or_create_custom_pipeline(RID shader) {

    auto it = custom_shader_pipelines.find(shader);

    if (it != custom_shader_pipelines.end()) {
        return it->second;
    }

    PipelineState pipeline_state;
    pipeline_state.shader                          = shader;
    pipeline_state.topology                        = PrimitiveTopology::TRIANGLES;
    pipeline_state.vertex_format.stride            = sizeof(Vertex);
    pipeline_state.vertex_format.attributes        = {{0, DataFormat::R32G32B32_SFLOAT, offsetof(Vertex, position)},
                                                      {1, DataFormat::R32G32B32A32_SFLOAT, offsetof(Vertex, color)},
                                                      {2, DataFormat::R32G32_SFLOAT, offsetof(Vertex, texcoord)}};
    pipeline_state.rasterization.cull_mode         = CullMode::NONE;
    pipeline_state.depth_stencil.depth_test_enable = false;

    RID pso = rd->pipeline_create(pipeline_state);

    custom_shader_pipelines[shader] = pso;

    return pipeline;
}

void RenderingCanvas::draw_custom(RID custom_shader, float x, float y, float width, float height, RID texture, const Color& color) {
    if (!is_drawing) {
        return;
    }

    flush();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + width / 2, y + height / 2, 0));
    transform           = glm::scale(transform, glm::vec3(width, height, 1));
    transform           = get_current_transform() * transform;

    Vector<Vertex> custom_vertices;
    Vector<uint16_t> custom_indices;

    glm::vec4 positions[] = {glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec4(0.5f, -0.5f, 0, 1), glm::vec4(0.5f, 0.5f, 0, 1),
                             glm::vec4(-0.5f, 0.5f, 0, 1)};
    glm::vec2 texcoords[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (int i = 0; i < 4; i++) {
        glm::vec4 pos = transform * positions[i];
        custom_vertices.push_back({glm::vec3(pos), color.to_vec4(), texcoords[i]});
    }

    custom_indices = {0, 1, 2, 2, 3, 0};


    PipelineState custom_pipeline_state;
    custom_pipeline_state.shader                   = custom_shader;
    custom_pipeline_state.topology                 = PrimitiveTopology::TRIANGLES;
    custom_pipeline_state.vertex_format.stride     = sizeof(Vertex);
    custom_pipeline_state.vertex_format.attributes = {{0, DataFormat::R32G32B32_SFLOAT, offsetof(Vertex, position)},
                                                      {1, DataFormat::R32G32B32A32_SFLOAT, offsetof(Vertex, color)},
                                                      {2, DataFormat::R32G32_SFLOAT, offsetof(Vertex, texcoord)}};

    custom_pipeline_state.rasterization.cull_mode         = CullMode::NONE;
    custom_pipeline_state.depth_stencil.depth_test_enable = false;

    BlendState blend;
    blend.enable    = true;
    blend.src_color = BlendFactor::SRC_ALPHA;
    blend.dst_color = BlendFactor::ONE_MINUS_SRC_ALPHA;
    blend.src_alpha = BlendFactor::ONE;
    blend.dst_alpha = BlendFactor::ONE_MINUS_SRC_ALPHA;
    custom_pipeline_state.blend_states.push_back(blend);

    RID custom_pipeline = rd->pipeline_create(custom_pipeline_state);

    rd->buffer_update(vertex_buffer, 0, custom_vertices.size() * sizeof(Vertex), custom_vertices.data());
    rd->buffer_update(index_buffer, 0, custom_indices.size() * sizeof(uint16_t), custom_indices.data());

    rd->bind_pipeline(custom_pipeline);
    rd->bind_vertex_buffers({vertex_buffer});
    rd->bind_index_buffer(index_buffer, IndexType::UINT16);

    glm::mat4 vp = projection * view;

    float time_value = SDL_GetTicks() / 1000.0f;
    rd->push_constant("TIME", &time_value, sizeof(float));

    if (texture != INVALID_RID) {
        rd->bind_texture(0, texture, default_sampler);
        int tex_unit = 0;
        rd->push_constant("TEXTURE", &tex_unit, sizeof(int));
    } else {
        rd->bind_texture(0, white_texture, default_sampler);
    }

    rd->draw_indexed(6, 1, 0, 0, 0);

    rd->pipeline_destroy(custom_pipeline);

    rd->bind_pipeline(pipeline);
}

void RenderingCanvas::draw_texture_ex(float x, float y, float width, float height, RID texture, const Rect& source, const Color& color,
                                      float rotation, bool flip_h, bool flip_v, const CanvasMaterial* material) {
    if (!is_drawing) {
        return;
    }

    RID final_texture     = texture;
    glm::vec4 final_color = color.to_vec4();

    if (material) {
        if (texture == INVALID_RID && material->get_texture() != INVALID_RID) {
            final_texture = material->get_texture();
        }

        final_color *= material->get_color().to_vec4();
    }

    bool use_custom_shader = material && material->has_custom_shader();

    if (use_custom_shader) {
        flush();
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x + width / 2.0f, y + height / 2.0f, 0));

    if (rotation != 0.0f) {
        transform = glm::rotate(transform, rotation, glm::vec3(0, 0, 1));
    }

    transform = glm::scale(transform, glm::vec3(width, height, 1));
    transform = get_current_transform() * transform;

    glm::vec4 positions[] = {glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec4(0.5f, -0.5f, 0, 1), glm::vec4(0.5f, 0.5f, 0, 1),
                             glm::vec4(-0.5f, 0.5f, 0, 1)};

    glm::vec2 texcoords[4];
    float u_min = 0.0f, v_min = 0.0f, u_max = 1.0f, v_max = 1.0f;

    if (source.width > 0 && source.height > 0 && final_texture != INVALID_RID) {
        uint32_t tex_width = 0, tex_height = 0;
        rd->get_texture_size(final_texture, tex_width, tex_height);

        if (tex_width > 0 && tex_height > 0) {
            u_min = source.x / (float) tex_width;
            v_min = source.y / (float) tex_height;
            u_max = (source.x + source.width) / (float) tex_width;
            v_max = (source.y + source.height) / (float) tex_height;
        }
    }

    if (flip_h) {
        float temp = u_min;
        u_min      = u_max;
        u_max      = temp;
    }

    if (flip_v) {
        float temp = v_min;
        v_min      = v_max;
        v_max      = temp;
    }

    texcoords[0] = {u_min, v_min};
    texcoords[1] = {u_max, v_min};
    texcoords[2] = {u_max, v_max};
    texcoords[3] = {u_min, v_max};

    if (use_custom_shader && material) {

        Vector<Vertex> custom_vertices;
        Vector<uint16_t> custom_indices;

        for (int i = 0; i < 4; i++) {
            glm::vec4 pos = transform * positions[i];
            custom_vertices.push_back({glm::vec3(pos), final_color, texcoords[i]});
        }

        custom_indices = {0, 1, 2, 2, 3, 0};

        RID shader_rid      = material->get_shader();
        RID custom_pipeline = INVALID_RID;

        auto it = custom_shader_pipelines.find(shader_rid);
        if (it != custom_shader_pipelines.end()) {
            custom_pipeline = it->second;
        } else {
            PipelineState custom_pipeline_state;
            custom_pipeline_state.shader                          = shader_rid;
            custom_pipeline_state.topology                        = PrimitiveTopology::TRIANGLES;
            custom_pipeline_state.vertex_format.stride            = sizeof(Vertex);
            custom_pipeline_state.vertex_format.attributes        = {{0, DataFormat::R32G32B32_SFLOAT, offsetof(Vertex, position)},
                                                                     {1, DataFormat::R32G32B32A32_SFLOAT, offsetof(Vertex, color)},
                                                                     {2, DataFormat::R32G32_SFLOAT, offsetof(Vertex, texcoord)}};
            custom_pipeline_state.rasterization.cull_mode         = CullMode::NONE;
            custom_pipeline_state.depth_stencil.depth_test_enable = false;

            BlendState blend;
            blend.enable    = true;
            blend.src_color = BlendFactor::SRC_ALPHA;
            blend.dst_color = BlendFactor::ONE_MINUS_SRC_ALPHA;
            blend.src_alpha = BlendFactor::ONE;
            blend.dst_alpha = BlendFactor::ONE_MINUS_SRC_ALPHA;
            custom_pipeline_state.blend_states.push_back(blend);

            custom_pipeline                     = rd->pipeline_create(custom_pipeline_state);
            custom_shader_pipelines[shader_rid] = custom_pipeline;

            spdlog::info("Created custom Pipeline for Shader RID={}", shader_rid);
        }

        rd->buffer_update(vertex_buffer, 0, custom_vertices.size() * sizeof(Vertex), custom_vertices.data());
        rd->buffer_update(index_buffer, 0, custom_indices.size() * sizeof(uint16_t), custom_indices.data());

        rd->bind_pipeline(custom_pipeline);
        rd->bind_vertex_buffers({vertex_buffer});
        rd->bind_index_buffer(index_buffer, IndexType::UINT16);

        glm::mat4 vp = projection * view;
        rd->push_constant("MODEL_MATRIX", glm::value_ptr(transform), sizeof(glm::mat4));
        rd->push_constant("VIEW_MATRIX", glm::value_ptr(view), sizeof(glm::mat4));
        rd->push_constant("PROJECTION_MATRIX", glm::value_ptr(projection), sizeof(glm::mat4));
        rd->push_constant("VIEW_PROJECTION_MATRIX", glm::value_ptr(vp), sizeof(glm::mat4));

        float time_value = SDL_GetTicks() / 1000.0f; /// TODO: get from engine DT
        rd->push_constant("TIME", &time_value, sizeof(float));

        int next_texture_unit = 1;

        for (const auto& [name, value] : material->get_uniforms()) {
            std::visit(
                [&]<typename T0>(T0&& val) {
                    using T = std::decay_t<T0>;
                    if constexpr (std::is_same_v<T, float>) {
                        rd->push_constant(name, &val, sizeof(float));
                    } else if constexpr (std::is_same_v<T, glm::vec2>) {
                        rd->push_constant(name, glm::value_ptr(val), sizeof(glm::vec2));
                    } else if constexpr (std::is_same_v<T, glm::vec3>) {
                        rd->push_constant(name, glm::value_ptr(val), sizeof(glm::vec3));
                    } else if constexpr (std::is_same_v<T, glm::vec4>) {
                        rd->push_constant(name, glm::value_ptr(val), sizeof(glm::vec4));
                    } else if constexpr (std::is_same_v<T, int>) {
                        rd->push_constant(name, &val, sizeof(int));
                    }
                },
                value);
        }

        for (const auto& [name, tex_rid] : material->get_custom_textures()) {
            if (tex_rid != INVALID_RID) {
                rd->bind_texture(next_texture_unit, tex_rid, default_sampler);
                rd->push_constant(name, &next_texture_unit, sizeof(int));
                next_texture_unit++;
            }
        }

        if (final_texture != INVALID_RID) {
            rd->bind_texture(0, final_texture, default_sampler);
            int tex_unit = 0;
            rd->push_constant("TEXTURE", &tex_unit, sizeof(int));
        } else {
            rd->bind_texture(0, white_texture, default_sampler);
        }

        rd->draw_indexed(6, 1, 0, 0, 0);

        rd->bind_pipeline(pipeline);
    } else {
        for (int i = 0; i < 4; i++) {
            glm::vec4 pos = transform * positions[i];
            vertices.push_back({glm::vec3(pos), final_color, texcoords[i]});
        }

        uint16_t base_index = (uint16_t) (vertices.size() - 4);
        indices.push_back(base_index + 0);
        indices.push_back(base_index + 1);
        indices.push_back(base_index + 2);
        indices.push_back(base_index + 2);
        indices.push_back(base_index + 3);
        indices.push_back(base_index + 0);

        RID tex = final_texture != INVALID_RID ? final_texture : white_texture;

        if (draw_commands.empty() || draw_commands.back().texture != tex) {
            DrawCommand cmd{};
            cmd.texture     = tex;
            cmd.index_count = 6;
            cmd.index_start = indices.size() - 6;
            cmd.use_texture = true;
            draw_commands.push_back(cmd);
        } else {
            draw_commands.back().index_count += 6;
        }
    }
}
