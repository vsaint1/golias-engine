#include "core/renderer/sdl/sdl_renderer.h"

#include "core/engine.h"


bool SDLRenderer::initialize(SDL_Window* window) {

    _window = window;
    if (!_window) {
        LOG_ERROR("SDLRenderer initialization failed: Window is null");
        return false;
    }


    _renderer = SDL_CreateRenderer(_window, nullptr);

    if (!_renderer) {
        LOG_ERROR("SDLRenderer initialization failed: %s", SDL_GetError());
        return false;
    }

    int driver_count = SDL_GetNumRenderDrivers();

    if (driver_count < 1) {
        LOG_ERROR("No render drivers available");
        return false;
    }

    std::string renderer_list;
    renderer_list.reserve(driver_count * 16);
    for (int i = 0; i < driver_count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        renderer_list += name;
        renderer_list += (i < driver_count - 1) ? ", " : "";
    }

    LOG_INFO("Available backends (%d): %s", driver_count, renderer_list.c_str());

    const char* renderer_name = SDL_GetRendererName(_renderer);

    const auto& viewport = GEngine->get_config().get_viewport();
    LOG_INFO("Using backend: %s, Viewport: %dx%d", renderer_name, viewport.width, viewport.height);
    SDL_SetRenderLogicalPresentation(_renderer, viewport.width, viewport.height, SDL_LOGICAL_PRESENTATION_STRETCH);

    // SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

    return true;
}


void SDLRenderer::clear(glm::vec4 color) {


    SDL_SetRenderDrawColor(_renderer, (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255));
    SDL_RenderClear(_renderer);
}

void SDLRenderer::flush(const glm::mat4& view, const glm::mat4& projection) {
    // TODO: flush geometry
}

void SDLRenderer::present() {

    SDL_RenderPresent(_renderer);
}


void SDLRenderer::draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string text = vformat(fmt, args);
    va_end(args);

    draw_text_internal(transform.world_position, color, font_name, text);
}

void SDLRenderer::draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) {

    SDL_SetRenderDrawColorFloat(_renderer, color.r, color.g, color.b, color.a);

    glm::vec2 start = transform.position;

    float cosr = cos(transform.rotation);
    float sinr = sin(transform.rotation);

    float x = end.x * transform.scale.x;
    float y = end.y * transform.scale.y;

    float rx = x * cosr - y * sinr;
    float ry = x * sinr + y * cosr;

    glm::vec2 final_pos = {transform.position.x + rx, transform.position.y + ry};

    SDL_RenderLine(_renderer, start.x, start.y, final_pos.x, final_pos.y);
}

void SDLRenderer::draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled) {


    if (SDL_abs(transform.rotation) < 0.001f) {
        SDL_FRect rect = {transform.position.x - (w * transform.scale.x) / 2.0f, transform.position.y - (h * transform.scale.y) / 2.0f,
                          w * transform.scale.x, h * transform.scale.y};

        if (is_filled) {
            SDL_RenderFillRect(_renderer, &rect);
        } else {
            SDL_RenderRect(_renderer, &rect);
        }
    } else {
        float hw = (w * transform.scale.x) / 2.0f;
        float hh = (h * transform.scale.y) / 2.0f;

        std::vector<glm::vec2> corners = {
            {-hw, -hh}, // Top-left
            {hw, -hh}, // Top-right
            {hw, hh}, // Bottom-right
            {-hw, hh} // Bottom-left
        };

        float cosr = cos(transform.rotation);
        float sinr = sin(transform.rotation);
        std::vector<SDL_FPoint> pts;

        for (const auto& corner : corners) {
            float rx = corner.x * cosr - corner.y * sinr;
            float ry = corner.x * sinr + corner.y * cosr;
            pts.push_back({transform.position.x + rx, transform.position.y + ry});
        }

        if (is_filled) {
            SDL_Vertex vertices[4];
            for (int i = 0; i < 4; ++i) {
                vertices[i].position  = pts[i];
                vertices[i].color     = SDL_FColor{color.r, color.g, color.b, color.a};
                vertices[i].tex_coord = {0.0f, 0.0f};
            }

            int indices[] = {0, 1, 2, 0, 2, 3};
            SDL_RenderGeometry(_renderer, nullptr, vertices, 4, indices, 6);
        } else {

            SDL_SetRenderDrawColorFloat(_renderer, color.r, color.g, color.b, color.a);
            pts.push_back(pts[0]);
            SDL_RenderLines(_renderer, pts.data(), pts.size());
        }
    }
}

void SDLRenderer::draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) {


    const float height                     = sqrt(3.0f) / 2.0f * size;
    std::vector<glm::vec2> triangle_points = {
        {0.0f, -height / 2.0f}, // Top vertex
        {-size / 2.0f, height / 2.0f}, // Bottom left
        {size / 2.0f, height / 2.0f} // Bottom right
    };

    float cosr = cos(transform.rotation);
    float sinr = sin(transform.rotation);
    std::vector<SDL_FPoint> pts;

    for (const auto& point : triangle_points) {
        float x  = point.x * transform.scale.x;
        float y  = point.y * transform.scale.y;
        float rx = x * cosr - y * sinr;
        float ry = x * sinr + y * cosr;
        pts.push_back({transform.position.x + rx, transform.position.y + ry});
    }

    if (is_filled) {
        SDL_Vertex vertices[3];
        for (int i = 0; i < 3; ++i) {
            vertices[i].position  = pts[i];
            vertices[i].color     = SDL_FColor{color.r, color.g, color.b, color.a};
            vertices[i].tex_coord = {0.0f, 0.0f};
        }

        int indices[] = {0, 1, 2};
        SDL_RenderGeometry(_renderer, nullptr, vertices, 3, indices, 3);
    } else {
        SDL_SetRenderDrawColorFloat(_renderer, color.r, color.g, color.b, color.a);
        pts.push_back(pts[0]);
        SDL_RenderLines(_renderer, pts.data(), pts.size());
    }
}


void SDLRenderer::draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) {
    if (points.empty()) {
        return;
    }


    std::vector<SDL_FPoint> pts;
    for (const auto& v : points) {
        float cosr = cos(transform.rotation), sinr = sin(transform.rotation);
        float x = v.x * transform.scale.x, y = v.y * transform.scale.y;
        float rx = x * cosr - y * sinr, ry = x * sinr + y * cosr;
        pts.push_back({transform.position.x + rx, transform.position.y + ry});
    }

    if (is_filled) {

        if (pts.size() >= 3) {
            std::vector<SDL_Vertex> vertices;
            std::vector<int> indices;

            for (size_t i = 0; i < pts.size(); ++i) {
                SDL_Vertex vertex;
                vertex.position  = pts[i];
                vertex.color     = SDL_FColor{color.r, color.g, color.b, color.a};
                vertex.tex_coord = {0.0f, 0.0f};
                vertices.push_back(vertex);
            }

            for (size_t i = 1; i < pts.size() - 1; ++i) {
                indices.push_back(0);
                indices.push_back(i);
                indices.push_back(i + 1);
            }

            SDL_RenderGeometry(_renderer, nullptr, vertices.data(), vertices.size(), indices.data(), indices.size());
        }
    } else {
        SDL_SetRenderDrawColorFloat(_renderer, color.r, color.g, color.b, color.a);
        pts.push_back(pts.front());
        SDL_RenderLines(_renderer, pts.data(), pts.size());
    }
}

std::vector<Tokens> SDLRenderer::parse_text(const std::string& text) {
    std::vector<Tokens> segments;
    auto cps = utf8_to_utf32(text);
    if (cps.empty()) {
        return segments;
    }

    std::string current;
    bool current_emoji = is_character_emoji(cps[0]);

    for (auto cp : cps) {
        bool is_e     = is_character_emoji(cp);
        std::string s = utf32_to_utf8(cp);
        if (is_e != current_emoji) {
            if (!current.empty()) {
                segments.push_back({current, current_emoji});
            }
            current       = s;
            current_emoji = is_e;
        } else {
            current += s;
        }
    }
    if (!current.empty()) {
        segments.push_back({current, current_emoji});
    }
    return segments;
}

bool SDLRenderer::load_font(const std::string& name, const std::string& path, int size) {

    if (_fonts.find(name) != _fonts.end()) {
        return true;
    }

    TTF_Font* f = TTF_OpenFont(path.c_str(), size);
    if (!f) {
        SDL_Log("Failed to load Font %s: %s", path.c_str(), SDL_GetError());
        return false;
    }

    _fonts[name] = std::make_unique<Font>(f);
    return true;
}

std::shared_ptr<Texture> SDLRenderer::load_texture(const std::string& name, const std::string& path,const aiTexture* ai_embedded_tex) {

    if (_textures.find(name) != _textures.end()) {
        return _textures[name];
    }

    FileAccess file(path, ModeFlags::READ);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open Texture file %s", path.c_str());
        return nullptr;
    }

    SDL_Surface* surface = IMG_Load_IO(file.get_handle(), false);

    if (!surface) {
        LOG_ERROR("Failed to load Texture %s: %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);

    if (!texture) {
        LOG_ERROR("Failed to create Texture from surface %s: %s", path.c_str(), SDL_GetError());
        SDL_DestroySurface(surface);

        return nullptr;
    }

    std::shared_ptr<SDLTexture> tex = std::make_shared<SDLTexture>(texture);
    tex->width                      = surface->w;
    tex->height                     = surface->h;
    tex->path                       = path;

    SDL_DestroySurface(surface);

    LOG_INFO("Loaded Texture Name: %s, Size (%dx%d), Path: %s", name.c_str(), tex->width, tex->height, tex->path.c_str());

    _textures[name] = tex;
    return _textures[name];
}

void SDLRenderer::draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name, const std::string& text) {
    auto segments = parse_text(text);
    float x       = pos.x;
    float y       = pos.y;

    for (auto& seg : segments) {
        Font* font = nullptr;
        if (!font_name.empty()) {
            font = seg.is_emoji ? _fonts[font_name].get() : _fonts[font_name].get();
        } else {
            font = seg.is_emoji ? _fonts[_emoji_font_name].get() : _fonts[_default_font_name].get();
        }

        if (!font || !font->get_font()) {
            continue;
        }

        SDL_Surface* surf = TTF_RenderText_Blended(font->get_font(), seg.text.c_str(), seg.text.size(),
                                                   SDL_Color{static_cast<Uint8>(color.r * 255), static_cast<Uint8>(color.g * 255),
                                                             static_cast<Uint8>(color.b * 255), static_cast<Uint8>(color.a * 255)});

        if (!surf) {
            continue;
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer, surf);

        if (!tex) {
            SDL_DestroySurface(surf);
            continue;
        }

        // SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

        SDL_FRect r{x, y, float(surf->w), float(surf->h)};
        SDL_RenderTexture(_renderer, tex, nullptr, &r);
        x += surf->w;

        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    }
}

void SDLRenderer::draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source,
                               bool flip_h = false, bool flip_v = false, const glm::vec4& color = glm::vec4(1, 1, 1, 1)) {


    if (!texture) {
        return;
    }

    SDLTexture* sdl_tex = dynamic_cast<SDLTexture*>(texture);

    if (!sdl_tex || !sdl_tex->get_texture()) {

        return;
    }

    SDL_FRect src_rect;
    src_rect.x = source.x;
    src_rect.y = source.y;
    src_rect.w = source.z;
    src_rect.h = source.w;

    SDL_FRect dst_rect;
    dst_rect.x = dest.x + transform.world_position.x;
    dst_rect.y = dest.y + transform.world_position.y;
    dst_rect.w = dest.z * transform.world_scale.x;
    dst_rect.h = dest.w * transform.world_scale.y;

    SDL_SetTextureColorModFloat(sdl_tex->get_texture(), color.r, color.g, color.b);
    SDL_SetTextureAlphaModFloat(sdl_tex->get_texture(), color.a);

    SDL_FPoint center;
    center.x = dst_rect.w * 0.5f;
    center.y = dst_rect.h * 0.5f;

    SDL_FlipMode flip = SDL_FLIP_NONE;
    if (flip_h && flip_v) {
        flip = (SDL_FlipMode) (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    } else if (flip_h) {
        flip = SDL_FLIP_HORIZONTAL;
    } else if (flip_v) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderTextureRotated(_renderer, sdl_tex->get_texture(), &src_rect, &dst_rect, transform.rotation, &center, flip);
}


void SDLRenderer::draw_circle(const Transform2D& transform, float radius, glm::vec4 color, bool is_filled) {

    float cx = transform.position.x + radius * transform.scale.x;
    float cy = transform.position.y + radius * transform.scale.y;

    if (is_filled) {
        std::vector<SDL_FPoint> points;
        int minX = (int) (cx - radius), maxX = (int) (cx + radius);
        int minY = (int) (cy - radius), maxY = (int) (cy + radius);

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                float dx = (x - cx) / radius;
                float dy = (y - cy) / radius;
                if (dx * dx + dy * dy <= 1.0f) {
                    points.push_back({(float) x, (float) y});
                }
            }
        }

        if (!points.empty()) {
            SDL_RenderPoints(_renderer, points.data(), points.size());
        }

    } else {

        SDL_SetRenderDrawColorFloat(_renderer, color.r, color.g, color.b, color.a);

        constexpr int segments = 64;
        std::vector<SDL_FPoint> pts;
        pts.reserve(segments + 1);

        for (int i = 0; i <= segments; i++) {
            float a = (i / (float) segments) * 2.0f * glm::pi<float>();
            pts.push_back({cx + cos(a) * radius, cy + sin(a) * radius});
        }

        SDL_RenderLines(_renderer, pts.data(), pts.size());
    }
}


SDLRenderer::~SDLRenderer() {

    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
}
