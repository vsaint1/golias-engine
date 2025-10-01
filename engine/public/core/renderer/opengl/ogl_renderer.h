#pragma once

#include "core/renderer/renderer.h"
#include "core/renderer/opengl/ogl_struct.h"


class OpenglRenderer final : public Renderer {
public:
    bool initialize(SDL_Window* window) override;

    void clear(glm::vec4 color) override;

    void present() override;


    bool load_font(const std::string& name, const std::string& path, int size) override;

    std::shared_ptr<Texture> load_texture(const std::string& name, const std::string& path) override;

    void draw_texture(const Transform2D& transform, Texture* texture, const glm::vec4& dest, const glm::vec4& source, bool flip_h,
                      bool flip_v, const glm::vec4& color) override;


    void draw_text(const Transform2D& transform, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) override;

    void draw_text_3d(const Transform3D& transform,const glm::mat4& view, const glm::mat4& projection, const glm::vec4& color, const std::string& font_name, const char* fmt, ...) override;

    void draw_rect(const Transform2D& transform, float w, float h, glm::vec4 color, bool is_filled) override;

    void draw_triangle(const Transform2D& transform, float size, glm::vec4 color, bool is_filled) override;

    void draw_line(const Transform2D& transform, glm::vec2 end, glm::vec4 color) override;

    void draw_circle(const Transform2D& transform, float radius, glm::vec4 color, bool is_filled) override;

    void draw_polygon(const Transform2D& transform, const std::vector<glm::vec2>& points, glm::vec4 color, bool is_filled) override;

    void draw_line_3d(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color) override;

    void draw_triangle_3d(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color, bool is_filled) override;

    ~OpenglRenderer() override;

    void draw_model(const Transform3D& t, const Model* model, const glm::mat4& view, const glm::mat4& projection,
                    const glm::vec3& viewPos) override;

    void draw_cube(const Transform3D& transform, const glm::mat4& view, const glm::mat4& proj, Uint32 shader) override;

    void draw_environment(const glm::mat4& view, const glm::mat4& projection) override;

    std::shared_ptr<Model> load_model(const char* path) override;

private:
    SDL_GLContext _context = nullptr;

    void setup_default_shaders();

    void setup_cubemap();

protected:
 
    std::unique_ptr<Mesh> load_mesh(aiMesh* mesh, const aiScene* scene, const std::string& base_dir);

    OpenglMesh* skybox_mesh = nullptr;

    OpenglShader* default_shader = nullptr;
    OpenglShader* skybox_shader = nullptr;
    
    std::vector<Tokens> parse_text(const std::string& text) override;

    void draw_text_internal(const glm::vec2& pos, const glm::vec4& color, const std::string& font_name, const std::string& text) override;
};
