#include "core/renderer/sdl_renderer.h"


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

    LOG_INFO("Available renderers (%d): %s", driver_count, renderer_list.c_str());

    const char* renderer_name = SDL_GetRendererName(_renderer);


    return true;
}


void SDLRenderer::clear(glm::vec4 color) {

    SDL_SetRenderDrawColor(_renderer, (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255), (Uint8) (color.a * 255));
    SDL_RenderClear(_renderer);
}

void SDLRenderer::present() {
    SDL_RenderPresent(_renderer);
}


void SDLRenderer::draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) {

    SDL_SetRenderDrawColor(_renderer, static_cast<Uint8>(color.r * 255), static_cast<Uint8>(color.g * 255),
                           static_cast<Uint8>(color.b * 255), static_cast<Uint8>(color.a * 255));

    glm::vec2 start = transform.position;

    float cosr = glm::cos(transform.rotation);
    float sinr = glm::sin(transform.rotation);

    float x = end.x * transform.scale.x;
    float y = end.y * transform.scale.y;

    float rx = x * cosr - y * sinr;
    float ry = x * sinr + y * cosr;

    glm::vec2 final_pos = {transform.position.x + rx, transform.position.y + ry};

    SDL_RenderLine(_renderer, start.x, start.y, final_pos.x, final_pos.y);
}

void SDLRenderer::draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled) {
    SDL_SetRenderDrawColor(_renderer, static_cast<Uint8>(color.r * 255), static_cast<Uint8>(color.g * 255),
                           static_cast<Uint8>(color.b * 255), static_cast<Uint8>(color.a * 255));

    if (abs(transform.rotation) < 0.001f) {
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
            pts.push_back(pts[0]);
            SDL_RenderLines(_renderer, pts.data(), pts.size());
        }
    }
}

void SDLRenderer::draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) {

    SDL_SetRenderDrawColor(_renderer, static_cast<Uint8>(color.r * 255), static_cast<Uint8>(color.g * 255),
                           static_cast<Uint8>(color.b * 255), static_cast<Uint8>(color.a * 255));


    float height                           = size * 0.866f; // sqrt(3)/2 for equilateral triangle
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
        pts.push_back(pts[0]);
        SDL_RenderLines(_renderer, pts.data(), pts.size());
    }
}


void SDLRenderer::draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) {
    if (points.empty()) {
        return;
    }

    SDL_SetRenderDrawColor(_renderer, static_cast<Uint8>(color.r * 255), static_cast<Uint8>(color.g * 255),
                           static_cast<Uint8>(color.b * 255), static_cast<Uint8>(color.a * 255));

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
        pts.push_back(pts.front());
        SDL_RenderLines(_renderer, pts.data(), pts.size());
    }
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

        SDL_SetRenderDrawColor(_renderer, (Uint8) (color.r * 255), (Uint8) (color.g * 255), (Uint8) (color.b * 255),
                               (Uint8) (color.a * 255));
        const int segments = 64;
        std::vector<SDL_FPoint> pts;
        pts.reserve(segments + 1);

        for (int i = 0; i <= segments; i++) {
            float a = (i / (float) segments) * 2.0f * M_PI;
            pts.push_back({cx + cos(a) * radius, cy + sin(a) * radius});
        }

        SDL_RenderLines(_renderer, pts.data(), pts.size());
    }
}


void SDLRenderer::shutdown() {

    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
}
