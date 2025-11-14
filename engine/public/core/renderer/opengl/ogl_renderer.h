#pragma once
#include  "ogl_struct.h"

class OpenGLRenderer final : public Renderer {


public:
    ~OpenGLRenderer() override;

    bool initialize(int w, int h, SDL_Window* window) override;

    std::shared_ptr<GpuImage> create_texture_2d(const std::string& path, const TextureDesc& desc) override;

    std::shared_ptr<GpuBuffer> allocate_gpu_buffer(GpuBufferType type) override;

    std::shared_ptr<GpuVertexLayout> create_vertex_layout(
        const GpuBuffer* vertex_buffer,
        const GpuBuffer* index_buffer,
        const std::vector<VertexAttribute>& attributes,
        Uint32 stride) override;

    Uint32 load_texture_from_file(const std::string& path) override;

    Uint32 load_embedded_texture(const unsigned char* buffer, size_t size, const std::string& name = "") override;

    void begin_frame() override;

    void begin_shadow_pass() override;

    void render_shadow_pass(const glm::mat4& light_space_matrix) override;

    void end_shadow_pass() override;

    void begin_render_target() override;

    void render_main_target(const Camera3D& camera,
                            const Transform3D& camera_transform,
                         const glm::mat4& light_space_matrix,
                         const std::vector<DirectionalLight3D>& directional_lights,
                         const std::vector<std::pair<Transform3D, SpotLight3D>>& spot_lights) override;

    void end_render_target() override;

    void begin_environment_pass() override;
    void render_environment_pass(const Camera3D& camera) override;
    void end_environment_pass() override;

    void add_to_render_batch(const Transform3D& transform, const MeshRef& mesh_ref, const MaterialRef& mat_ref) override;

    void add_to_shadow_batch(const Transform3D& transform, const MeshRef& mesh_ref) override;


    void set_render_resolution(int width, int height) override;

    void cleanup() override;

    void swap_chain() override;

    // ========================================================================
    // 2D Rendering Implementation
    // ========================================================================
    void begin_frame_2d() override;
    void end_frame_2d(const Camera2D& camera, const Transform2D& camera_transform) override;
    
    void draw_rect_2d(const glm::vec2& position, const glm::vec2& size, 
                     const glm::vec4& color, bool filled = true) override;
    
    void draw_texture_2d(Uint32 texture_id, 
                        const glm::vec2& position, 
                        const glm::vec2& size,
                        const Rect2D& src = {0,0,0,0},
                        FlipMode flip = FlipMode::NONE,
                        const glm::vec4& color = glm::vec4(1.0f)) override;
    
    void draw_line_2d(const glm::vec2& start, const glm::vec2& end, 
                     const glm::vec4& color, float thickness = 1.0f) override;
    
    void draw_circle_2d(const glm::vec2& center, float radius, 
                       const glm::vec4& color, bool filled = true, 
                       int segments = 32) override;
    
    void draw_circle_outline_2d(const glm::vec2& center, float radius, 
                               const glm::vec4& color, float thickness = 1.0f, 
                               int segments = 32) override;
    
    void draw_triangle_2d(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, 
                         const glm::vec4& color, bool filled = true) override;
    
    void draw_text_2d(const std::string& text, const glm::vec2& position, 
                     const glm::vec4& color, float scale = 1.0f) override;
    
    bool load_font(const std::string& font_path, int point_size, const std::string& font_name = "") override;
    

private:
    SDL_GLContext _context = nullptr;

    GLuint create_gl_texture(const unsigned char* data, int w, int h, int channels);

    WorldEnvironment3D* create_skybox_from_atlas(const std::string& atlas_path,
                                               CubemapOrientation orient = CubemapOrientation::DEFAULT,
                                               float brightness          = 1.0f);


    void setup_instance_matrix_attribute(GpuVertexLayout* vao);


    void setup_lights(const std::vector<DirectionalLight3D>& directional_lights,
                      const std::vector<std::pair<Transform3D, SpotLight3D>>& spot_lights);

    // Rendering passes for material blending
    void render_opaque_pass();
    void render_transparent_pass();

    void initialize_2d_rendering();

    void render_batched_2d(DrawMode2D mode, GLuint texture_id, bool use_texture,
                          const DrawCommand2D& first_cmd);
    
    // Render text to texture (always white, color applied at render time)
    GLuint render_text_to_texture(const std::string& text, TTF_Font* font);
};

