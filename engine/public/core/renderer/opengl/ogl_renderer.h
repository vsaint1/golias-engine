#pragma once
#include  "ogl_struct.h"

class OpenGLRenderer final : public Renderer {


public:
    ~OpenGLRenderer() override;

    bool initialize(int w, int h, SDL_Window* window) override;

    std::shared_ptr<GpuBuffer> allocate_gpu_buffer(GpuBufferType type) override;

    std::shared_ptr<GpuVertexLayout> create_vertex_layout(
        const GpuBuffer* vertex_buffer,
        const GpuBuffer* index_buffer,
        const std::vector<VertexAttribute>& attributes,
        uint32_t stride) override;

    Uint32 load_texture_from_file(const std::string& path) override;

    Uint32 load_embedded_texture(const unsigned char* buffer, size_t size, const std::string& name = "") override;

    Uint32 load_texture_from_raw_data(const unsigned char* data, int w, int h, int channels = 4, const std::string& name = "") override;

    void begin_frame() override;

    void begin_shadow_pass() override;

    void render_shadow_pass(const glm::mat4& light_space_matrix) override;

    void end_shadow_pass() override;

    void begin_render_target() override;

    void render_main_target(const Camera3D& camera,
                            const Transform3D& camera_transform,
                         const glm::mat4& light_space_matrix,
                         const std::vector<DirectionalLight>& directional_lights,
                         const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights) override;

    void end_render_target() override;

    void begin_environment_pass() override;
    void render_environment_pass(const Camera3D& camera) override;
    void end_environment_pass() override;

    void add_to_render_batch(const Transform3D& transform, const MeshRef& mesh_ref, const MaterialRef& mat_ref) override;

    void add_to_shadow_batch(const Transform3D& transform, const MeshRef& mesh_ref) override;

    void resize(int w, int h) override;

    void cleanup() override;

    void swap_chain() override;

private:
    SDL_GLContext _context = nullptr;

    GLuint create_gl_texture(const unsigned char* data, int w, int h, int channels);

    WorldEnvironment* create_skybox_from_atlas(const std::string& atlas_path,
                                               CubemapOrientation orient = CubemapOrientation::DEFAULT,
                                               float brightness          = 1.0f);


    void setup_instance_matrix_attribute(GpuVertexLayout* vao);


    void setup_lights(const std::vector<DirectionalLight>& directional_lights,
                      const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights);

};
