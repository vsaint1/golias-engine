#pragma once
#include  "ogl_struct.h"

class OpenGLRenderer final : public Renderer {
    SDL_GLContext _context = nullptr;

    static GLuint create_gl_texture(const unsigned char* data, int w, int h, int channels);

public:
    ~OpenGLRenderer() override;

    bool initialize(int w, int h, SDL_Window* window) override;

    Uint32 load_texture_from_file(const std::string& path) override;

    Uint32 load_texture_from_memory(const unsigned char* buffer, size_t size, const std::string& name = "") override;

    Uint32 load_texture_from_raw_data(const unsigned char* data, int w, int h, int channels = 4, const std::string& name = "") override;

    void begin_shadow_pass() override;

    void render_shadow_pass(const Transform3D& transform, const MeshInstance3D& mesh, const glm::mat4& light_space_matrix) override;

    void end_shadow_pass() override;

    void begin_render_target() override;

    void render_entity(const Transform3D& transform,
                       const MeshInstance3D& mesh,
                       const Material& material,
                       const Camera3D& camera,
                       const glm::mat4& light_space_matrix,
                       const std::vector<DirectionalLight>& directional_lights,
                       const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights) override;

    void end_render_target() override;

    void resize(int w, int h) override;

    void cleanup() override;

    void swap_chain() override;
};
