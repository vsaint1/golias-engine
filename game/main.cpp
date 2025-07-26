#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1366;
int SCREEN_HEIGHT = 768;


// class Renderer2D {
//     GLuint vao, vbo, ebo;
//     GLuint shader_program;
//     GLuint projection_uniform;
//     FT_Library ft = {};
//     FT_Face face;
//     std::unordered_map<std::string, Font> fonts;
//     std::string current_font_name;
//     int screen_width, screen_height;
//     glm::mat4 projection;
//     std::vector<DrawCommand> commands;
//     std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
//     SDL_Window* _window = nullptr;
//
//     const char* vertex_shader_source = R"(
//         #version 330 core
//         layout (location = 0) in vec2 a_pos;
//         layout (location = 1) in vec2 a_tex_coord;
//         layout (location = 2) in vec4 a_color;
//         uniform mat4 projection;
//         out vec2 tex_coord;
//         out vec4 color;
//         void main() {
//             gl_Position = projection * vec4(a_pos, 0.0, 1.0);
//             tex_coord = a_tex_coord;
//             color = a_color;
//         }
//     )";
//
//     const char* fragment_shader_source = R"(
//         #version 330 core
//         in vec2 tex_coord;
//         in vec4 color;
//         uniform sampler2D our_texture;
//         uniform bool use_texture;
//         out vec4 COLOR;
//         void main() {
//             if (use_texture) {
//                 vec4 tex_color = texture(our_texture, tex_coord);
//                 COLOR = tex_color * color;
//             } else {
//                 COLOR = color;
//             }
//         }
//     )";
//
// public:
//     Renderer2D(int width, int height, SDL_Window* window) : screen_width(width), screen_height(height), _window(window) {
//         init_opengl();
//         init_freetype();
//         projection = glm::ortho(0.0f, (float) width, (float) height, 0.0f, -1.0f, 1.0f);
//     }
//     ~Renderer2D() {
//         cleanup();
//     }
//
//     void init_opengl() {
//
//     }
//
//     void init_freetype() {
//
//     }
//
//     GLuint compile_shader(GLenum type, const char* source) {
//         GLuint shader = glCreateShader(type);
//         glShaderSource(shader, 1, &source, NULL);
//         glCompileShader(shader);
//         int success;
//         glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//         if (!success) {
//             char info_log[512];
//             glGetShaderInfoLog(shader, 512, NULL, info_log);
//             std::cerr << "Shader compilation failed: " << info_log << std::endl;
//         }
//         return shader;
//     }
//
//     void setup_camera(const Camera2D& camera) {
//         projection = camera.get_projection_matrix() * camera.get_view_matrix();
//     }
//
//     void setup_canvas(const int width, const int height) {
//         projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
//     }
//
//     bool load_font(const std::string& font_path, const std::string& font_alias, int font_size = 48) {
//         if (FT_New_Face(ft, font_path.c_str(), 0, &face)) {
//             std::cerr << "Failed to load font: " << font_path << std::endl;
//             return false;
//         }
//         FT_Set_Pixel_Sizes(face, 0, font_size);
//         Font font;
//         font.font_path = font_path;
//         font.font_size = font_size;
//         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//         for (unsigned char c = 0; c < 128; c++) {
//             if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//                 std::cerr << "Failed to load Glyph: " << (char) c << std::endl;
//                 continue;
//             }
//             int w                 = face->glyph->bitmap.width;
//             int h                 = face->glyph->bitmap.rows;
//             unsigned char* buffer = face->glyph->bitmap.buffer;
//             std::vector<unsigned char> rgba_buffer(w * h * 4);
//             for (int i = 0; i < w * h; ++i) {
//                 rgba_buffer[4 * i + 0] = 255;
//                 rgba_buffer[4 * i + 1] = 255;
//                 rgba_buffer[4 * i + 2] = 255;
//                 rgba_buffer[4 * i + 3] = buffer[i];
//             }
//             GLuint texture;
//             glGenTextures(1, &texture);
//             glBindTexture(GL_TEXTURE_2D, texture);
//             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//             Character character = {texture, glm::ivec2(w, h), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//                                    static_cast<GLuint>(face->glyph->advance.x)};
//             font.characters.insert(std::pair<char, Character>(c, character));
//         }
//         glBindTexture(GL_TEXTURE_2D, 0);
//         fonts[font_alias] = font;
//         if (current_font_name.empty()) {
//             current_font_name = font_alias;
//         }
//         return true;
//     }
//
//     void set_current_font(const std::string& font_name) {
//         if (fonts.find(font_name) != fonts.end()) {
//             current_font_name = font_name;
//         } else {
//             std::cerr << "Font not found: " << font_name << std::endl;
//         }
//     }
//
//     Texture& load_texture(const std::string& path) {
//         auto it = textures.find(path);
//         if (it != textures.end()) {
//             return *(it->second);
//         }
//         auto texture = std::make_unique<Texture>();
//         glGenTextures(1, &texture->id);
//         int nr_channels;
//         stbi_set_flip_vertically_on_load(false);
//         unsigned char* data = stbi_load(path.c_str(), &texture->width, &texture->height, &nr_channels, 0);
//         if (data) {
//             GLenum format = GL_RGB;
//             if (nr_channels == 1) {
//                 format = GL_RED;
//             } else if (nr_channels == 3) {
//                 format = GL_RGB;
//             } else if (nr_channels == 4) {
//                 format = GL_RGBA;
//             }
//             glBindTexture(GL_TEXTURE_2D, texture->id);
//             glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);
//             glGenerateMipmap(GL_TEXTURE_2D);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//             stbi_image_free(data);
//             textures[path] = std::move(texture);
//             return *(textures[path]);
//         } else {
//             std::cerr << "Failed to load texture: " << path << std::endl;
//             std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
//             glDeleteTextures(1, &texture->id);
//             static Texture dummy;
//             return dummy;
//         }
//     }
//
//     Texture& get_texture(const std::string& path) {
//         auto it = textures.find(path);
//         if (it != textures.end()) {
//             return *(it->second);
//         }
//         return load_texture(path);
//     }
//
//     void draw_texture(const Texture& texture, const Rect2& dest_rect,float rotation, const glm::vec4& color = glm::vec4(1.0f),
//                       const Rect2& src_rect = {0, 0, 0, 0}, int z_index = 0 ) {
//         if (!texture.id) {
//             return;
//         }
//         float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
//
//         if (!src_rect.is_zero()) {
//             u0 = src_rect.x / texture.width;
//             v0 = src_rect.y / texture.height;
//             u1 = (src_rect.x + src_rect.width) / texture.width;
//             v1 = (src_rect.y + src_rect.height) / texture.height;
//         }
//
//         BatchKey key{texture.id, z_index, DrawCommandType::TEXTURE};
//         add_quad_to_batch(key, dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height, u0, v0, u1, v1, color, rotation);
//
//
//     }
//
//     void draw_rect(Rect2 rect,  float rotation,const glm::vec4& color, bool filled = true, int z_index = 0) {
//         if (filled) {
//             BatchKey key{0, z_index, DrawCommandType::RECT};
//             add_quad_to_batch(key, rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, color, rotation);
//         } else {
//             float line_width = 1.0f;
//             glm::vec2 center(rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f);
//             glm::vec2 p1 = rotate_point({rect.x, rect.y}, center, rotation);
//             glm::vec2 p2 = rotate_point({rect.x + rect.width, rect.y}, center, rotation);
//             glm::vec2 p3 = rotate_point({rect.x + rect.width, rect.y + rect.height}, center, rotation);
//             glm::vec2 p4 = rotate_point({rect.x, rect.y + rect.height}, center, rotation);
//             draw_line(p1.x, p1.y, p2.x, p2.y, line_width, color, z_index);
//             draw_line(p2.x, p2.y, p3.x, p3.y, line_width, color, z_index);
//             draw_line(p3.x, p3.y, p4.x, p4.y, line_width, color, z_index);
//             draw_line(p4.x, p4.y, p1.x, p1.y, line_width, color, z_index);
//         }
//     }
//
//     void draw_text(const std::string& text, float x, float y, float rotation,float scale, const glm::vec4& color, const std::string& font_alias = "",
//                    int z_index = 0) {
//
//         const std::string use_font_name = font_alias.empty() ? current_font_name : font_alias;
//
//         if (use_font_name.empty() || !fonts.contains(use_font_name)) {
//             return;
//         }
//
//         const Font& font = fonts[use_font_name];
//
//         float xpos = x;
//         float min_x = x, max_x = x, min_y = y, max_y = y;
//         for (char c : text) {
//             auto it = font.characters.find(c);
//
//             if (it == font.characters.end()) {
//                 continue;
//             }
//
//             const Character& ch = it->second;
//             float w             = ch.size.x * scale;
//             float h             = ch.size.y * scale;
//             float x0            = xpos + ch.bearing.x * scale;
//             float y0            = y + (font.font_size - ch.bearing.y) * scale;
//             min_x               = SDL_min(min_x, x0);
//             max_x               = SDL_max(max_x, x0 + w);
//             min_y               = SDL_min(min_y, y0);
//             max_y               = SDL_max(max_y, y0 + h);
//             xpos += (ch.advance >> 6) * scale;
//         }
//
//         const glm::vec2 text_center((min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f);
//
//         xpos = x;
//         for (char c : text) {
//             auto it = font.characters.find(c);
//             if (it == font.characters.end()) continue;
//             const Character& ch = it->second;
//             float w = ch.size.x * scale;
//             float h = ch.size.y * scale;
//             float x0 = xpos + ch.bearing.x * scale;
//             float y0 = y + (font.font_size - ch.bearing.y) * scale;
//
//             glm::vec2 glyph_pos = rotate_point({x0, y0}, text_center, rotation);
//
//             BatchKey key{ch.texture_id, z_index, DrawCommandType::TEXT};
//             add_quad_to_batch(key, glyph_pos.x, glyph_pos.y, w, h, 0, 0, 1, 1, color, 0.0f);
//             xpos += (ch.advance >> 6) * scale;
//         }
//     }
//
//     void draw_line(float x1, float y1, float x2, float y2, float width, const glm::vec4& color, int z_index = 0, float rotation = 0.0f) {
//         glm::vec2 dir    = glm::normalize(glm::vec2(x2 - x1, y2 - y1));
//         glm::vec2 normal = glm::vec2(-dir.y, dir.x) * (width * 0.5f);
//         glm::vec2 center = (glm::vec2(x1, y1) + glm::vec2(x2, y2)) * 0.5f;
//         glm::vec2 p1     = rotate_point({x1 - normal.x, y1 - normal.y}, center, rotation);
//         glm::vec2 p2     = rotate_point({x1 + normal.x, y1 + normal.y}, center, rotation);
//         glm::vec2 p3     = rotate_point({x2 + normal.x, y2 + normal.y}, center, rotation);
//         glm::vec2 p4     = rotate_point({x2 - normal.x, y2 - normal.y}, center, rotation);
//
//         BatchKey key{0, z_index, DrawCommandType::LINE};
//         Batch& batch  = batches[key];
//         batch.z_index = z_index;
//         uint32_t base = batch.vertices.size();
//         batch.vertices.push_back({p1, {0.0f, 0.0f}, color});
//         batch.vertices.push_back({p2, {0.0f, 0.0f}, color});
//         batch.vertices.push_back({p3, {0.0f, 0.0f}, color});
//         batch.vertices.push_back({p4, {0.0f, 0.0f}, color});
//         batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
//     }
//
//     void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3, const glm::vec4& color, bool filled = true,
//                        int z_index = 0, float rotation = 0.0f) {
//         glm::vec2 center = (glm::vec2(x1, y1) + glm::vec2(x2, y2) + glm::vec2(x3, y3)) / 3.0f;
//         glm::vec2 p1     = rotate_point({x1, y1}, center, rotation);
//         glm::vec2 p2     = rotate_point({x2, y2}, center, rotation);
//         glm::vec2 p3     = rotate_point({x3, y3}, center, rotation);
//
//         if (filled) {
//             BatchKey key{0, z_index, DrawCommandType::TRIANGLE};
//             Batch& batch  = batches[key];
//             batch.z_index = z_index;
//             uint32_t base = batch.vertices.size();
//             batch.vertices.push_back({p1, {0.0f, 0.0f}, color});
//             batch.vertices.push_back({p2, {0.0f, 0.0f}, color});
//             batch.vertices.push_back({p3, {0.0f, 0.0f}, color});
//             batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2});
//         } else {
//             float line_width = 1.0f;
//             draw_line(p1.x, p1.y, p2.x, p2.y, line_width, color, z_index);
//             draw_line(p2.x, p2.y, p3.x, p3.y, line_width, color, z_index);
//             draw_line(p3.x, p3.y, p1.x, p1.y, line_width, color, z_index);
//         }
//     }
//
//     void draw_circle(float center_x, float center_y, float rotation, float radius, const glm::vec4& color, bool filled = true, int segments = 32,
//                      int z_index = 0) {
//         glm::vec2 center(center_x, center_y);
//         if (filled) {
//             BatchKey key{0, z_index, DrawCommandType::CIRCLE};
//             Batch& batch  = batches[key];
//             batch.z_index = z_index;
//             uint32_t base = batch.vertices.size();
//             batch.vertices.push_back({center, {0.5f, 0.5f}, color});
//             for (int i = 0; i <= segments; ++i) {
//                 float angle = 2.0f * M_PI * i / segments;
//                 float x     = center_x + radius * cos(angle);
//                 float y     = center_y + radius * sin(angle);
//                 glm::vec2 p = rotate_point({x, y}, center, rotation);
//                 batch.vertices.push_back({p, {0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)}, color});
//                 if (i > 0) {
//                     batch.indices.push_back(base);
//                     batch.indices.push_back(base + i);
//                     batch.indices.push_back(base + i + 1);
//                 }
//             }
//         } else {
//             float line_width = 1.0f;
//             for (int i = 0; i < segments; ++i) {
//                 float angle1 = 2.0f * M_PI * i / segments;
//                 float angle2 = 2.0f * M_PI * (i + 1) / segments;
//                 glm::vec2 p1 = rotate_point({center_x + radius * cos(angle1), center_y + radius * sin(angle1)}, center, rotation);
//                 glm::vec2 p2 = rotate_point({center_x + radius * cos(angle2), center_y + radius * sin(angle2)}, center, rotation);
//                 draw_line(p1.x, p1.y, p2.x, p2.y, line_width, color, z_index);
//             }
//         }
//     }
//
//     void draw_polygon(const std::vector<glm::vec2>& points,float rotation, const glm::vec4& color, bool filled = true, int z_index = 0) {
//         if (points.size() < 3) {
//             return;
//         }
//         glm::vec2 center(0.0f);
//         for (const auto& p : points) {
//             center += p;
//         }
//         center /= static_cast<float>(points.size());
//
//         std::vector<glm::vec2> rotated_points;
//         for (const auto& p : points) {
//             rotated_points.push_back(rotate_point(p, center, rotation));
//         }
//
//         if (filled) {
//             BatchKey key{0, z_index, DrawCommandType::POLYGON};
//             Batch& batch  = batches[key];
//             batch.z_index = z_index;
//             uint32_t base = batch.vertices.size();
//             for (const auto& point : rotated_points) {
//                 batch.vertices.push_back({point, {0.0f, 0.0f}, color});
//             }
//             for (size_t i = 1; i < rotated_points.size() - 1; ++i) {
//                 batch.indices.push_back(base);
//                 batch.indices.push_back(base + i);
//                 batch.indices.push_back(base + i + 1);
//             }
//         } else {
//             float line_width = 1.0f;
//             for (size_t i = 0; i < rotated_points.size(); ++i) {
//                 size_t next = (i + 1) % rotated_points.size();
//                 draw_line(rotated_points[i].x, rotated_points[i].y, rotated_points[next].x, rotated_points[next].y, line_width, color,
//                           z_index);
//             }
//         }
//     }
//
//     void clear(const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
//         glClearColor(color.r, color.g, color.b, color.a);
//         glClear(GL_COLOR_BUFFER_BIT);
//         commands.clear();
//         batches.clear();
//     }
//
//     void flush() {
//
//     }
//
// private:
//     std::unordered_map<BatchKey, Batch> batches;
//
//     glm::vec2 rotate_point(const glm::vec2& point, const glm::vec2& center, float rotation) {
//         float s           = sin(rotation);
//         float c           = cos(rotation);
//         glm::vec2 rel     = point - center;
//         glm::vec2 rot_rel = glm::vec2(rel.x * c - rel.y * s, rel.x * s + rel.y * c);
//         return center + rot_rel;
//     }
//
//     void add_quad_to_batch(const BatchKey& key, float x, float y, float w, float h, float u0, float v0, float u1, float v1,
//                            const glm::vec4& color, float rotation = 0.0f) {
//         Batch& batch     = batches[key];
//         batch.texture_id = key.texture_id;
//         batch.type       = key.type;
//         batch.z_index    = key.z_index;
//         uint32_t base    = batch.vertices.size();
//
//         glm::vec2 center = glm::vec2(x + w * 0.5f, y + h * 0.5f);
//
//         batch.vertices.push_back({rotate_point({x, y + h}, center, rotation), {u0, v1}, color});
//         batch.vertices.push_back({rotate_point({x + w, y + h}, center, rotation), {u1, v1}, color});
//         batch.vertices.push_back({rotate_point({x + w, y}, center, rotation), {u1, v0}, color});
//         batch.vertices.push_back({rotate_point({x, y}, center, rotation), {u0, v0}, color});
//         batch.indices.insert(batch.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
//     }
//
//     void render_command(const DrawCommand& cmd) {
//         if (cmd.vertices.empty() || cmd.type == DrawCommandType::NONE) {
//             return;
//         }
//         glBindBuffer(GL_ARRAY_BUFFER, vbo);
//         glBufferData(GL_ARRAY_BUFFER, cmd.vertices.size() * sizeof(Vertex), cmd.vertices.data(), GL_DYNAMIC_DRAW);
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, cmd.indices.size() * sizeof(uint32_t), cmd.indices.data(), GL_DYNAMIC_DRAW);
//         bool use_texture = (cmd.type == DrawCommandType::TEXTURE || cmd.type == DrawCommandType::TEXT) && cmd.texture_id != 0;
//         glUniform1i(glGetUniformLocation(shader_program, "use_texture"), use_texture);
//         if (use_texture) {
//             glActiveTexture(GL_TEXTURE0);
//             glBindTexture(GL_TEXTURE_2D, cmd.texture_id);
//             glUniform1i(glGetUniformLocation(shader_program, "our_texture"), 0);
//         }
//         if (cmd.type == DrawCommandType::LINE) {
//             glLineWidth(cmd.line_width);
//         }
//         glDrawElements(GL_TRIANGLES, cmd.indices.size(), GL_UNSIGNED_INT, 0);
//         if (use_texture) {
//             glBindTexture(GL_TEXTURE_2D, 0);
//         }
//     }
//
//     void cleanup() {
//         glDeleteVertexArrays(1, &vao);
//         glDeleteBuffers(1, &vbo);
//         glDeleteBuffers(1, &ebo);
//         glDeleteProgram(shader_program);
//
//         if (face) {
//             FT_Done_Face(face);
//         }
//
//         FT_Done_FreeType(ft);
//
//
//         for (auto& font_pair : fonts) {
//             for (auto& char_pair : font_pair.second.characters) {
//                 glDeleteTextures(1, &char_pair.second.texture_id);
//             }
//         }
//     }
// };

int main(int argc, char* argv[]) {

    if (!GEngine->initialize("Example - new API", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }


    if (!GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", "mine", 48)) {
        LOG_ERROR("failed to load mine font");
        return SDL_APP_FAILURE;

    }

    if (!GEngine->get_renderer()->load_font("fonts/Arial.otf", "arial", 48)) {
        LOG_ERROR("failed to load arial font");
        return SDL_APP_FAILURE;
    }

    Texture sample_texture  = GEngine->get_renderer()->load_texture("sprites/Character_001.png");
    Texture sample_texture2 = GEngine->get_renderer()->load_texture("sprites/Character_002.png");

    bool quit                = false;
    SDL_Event e;
    float angle = 0.0f;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        angle += 0.01f;
        if (angle > 2 * M_PI) {
            angle -= 2 * M_PI;
        }

        GEngine->get_renderer()->clear(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        GEngine->get_renderer()->draw_rect({100, 100, 200, 150}, angle, glm::vec4(1.0f, 0.5f, 0.2f, 1.0f), true, 1);
        GEngine->get_renderer()->draw_circle(400, 300, 0, 80, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), false, 32, 2);
        GEngine->get_renderer()->draw_triangle(500, 100, 600, 200, 450, 200,0, glm::vec4(0.8f, 0.2f, 0.8f, 1.0f), false, 3);
        GEngine->get_renderer()->draw_line(50, 50, 750, 550, 3.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 5);
        std::vector<glm::vec2> polygon_points = {{200, 400}, {250, 380}, {300, 420}, {280, 480}, {220, 490}};
        GEngine->get_renderer()->draw_polygon(polygon_points, 0, glm::vec4(0.5f, 0.5f, 1.0f, 1.0f), true, 1);


        for (int i = 0; i < 100; ++i) {
            GEngine->get_renderer()->draw_texture(sample_texture, {i * 50.f, i * 50.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                                  {0, 0, 64, 64}, 0);
        }

        GEngine->get_renderer()->draw_texture(sample_texture2, {400, 200, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {}, 5);
        GEngine->get_renderer()->draw_text("Hola amigo, como estas?", 100, 60, angle, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine", 10);
        GEngine->get_renderer()->draw_text("Ola mundo", 100, 250, 0, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "arial", 10);
        GEngine->get_renderer()->draw_text("Hello world", 100, 350, 0, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine", 10);
        GEngine->get_renderer()->draw_text("No internationalization  =(", 100, 500, 0, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine", 10);


        GEngine->get_renderer()->flush();
    }


    GEngine->shutdown();

    return 0;
}
