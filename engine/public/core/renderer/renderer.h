#pragma once
#include  "core/component/components.h"
#include "base_struct.h"

constexpr int MAX_INSTANCES = 1000;

class Renderer {
public:
    virtual ~Renderer() = default;

    virtual bool initialize(int width, int height, SDL_Window* window) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void cleanup() = 0;

    virtual std::shared_ptr<GpuBuffer> allocate_gpu_buffer(GpuBufferType type) = 0;

    virtual std::shared_ptr<GpuVertexLayout> create_vertex_layout(
        const GpuBuffer* vertex_buffer,
        const GpuBuffer* index_buffer,
        const std::vector<VertexAttribute>& attributes,
        uint32_t stride) = 0;

    virtual Uint32 load_texture_from_file(const std::string& path) = 0;

    virtual Uint32 load_embedded_texture(const unsigned char* buffer, size_t size, const std::string& name = "") = 0;

    virtual void begin_frame() = 0;

    virtual void begin_shadow_pass() = 0;
    virtual void render_shadow_pass(const glm::mat4& light_space_matrix) = 0;
    virtual void end_shadow_pass() = 0;

    virtual void begin_render_target() = 0;
    virtual void render_main_target(const Camera3D& camera,
                                    const Transform3D& camera_transform,
                                    const glm::mat4& light_space_matrix,
                                    const std::vector<DirectionalLight>& directional_lights,
                                    const std::vector<std::pair<Transform3D, SpotLight>>& spot_lights) = 0;
    virtual void end_render_target() = 0;

    virtual void begin_environment_pass() =0;
    virtual void render_environment_pass(const Camera3D& camera) =0;
    virtual void end_environment_pass() =0;

    virtual void add_to_render_batch(const Transform3D& transform,
                                     const MeshRef& mesh_ref, const MaterialRef& mat_ref) = 0;

    virtual void add_to_shadow_batch(const Transform3D& transform, const MeshRef& mesh_ref) = 0;


    virtual void swap_chain() = 0;


    // ========================================================================
    // 2D Rendering API
    // ========================================================================
    virtual void set_2d_virtual_resolution(int width, int height) = 0;

    virtual void begin_frame_2d() = 0;
    virtual void end_frame_2d(const Camera2D& camera, const Transform2D& camera_transform) = 0;

    virtual void draw_rect_2d(const glm::vec2& position, const glm::vec2& size,
                              const glm::vec4& color, bool filled = true) = 0;

    virtual void draw_texture_2d(Uint32 texture_id,
                                 const glm::vec2& position,
                                 const glm::vec2& size,
                                 const Rect2D& src     = {0, 0, 0, 0},
                                 FlipMode flip         = FlipMode::NONE,
                                 const glm::vec4& tint = glm::vec4(1.0f)) = 0;

    virtual void draw_line_2d(const glm::vec2& start, const glm::vec2& end,
                              const glm::vec4& color, float thickness = 1.0f) = 0;

    virtual void draw_circle_2d(const glm::vec2& center, float radius,
                                const glm::vec4& color, bool filled = true,
                                int segments                        = 32) = 0;

    virtual void draw_circle_outline_2d(const glm::vec2& center, float radius,
                                        const glm::vec4& color, float thickness = 1.0f,
                                        int segments                            = 32) = 0;

    virtual void draw_triangle_2d(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                                  const glm::vec4& color, bool filled = true) = 0;

    virtual void draw_text_2d(const std::string& text, const glm::vec2& position,
                              const glm::vec4& color, float scale = 1.0f) = 0;

    template <typename... Args>
    void draw_text_2d_fmt(const glm::vec2& position, const glm::vec4& color, float scale,
                          std::format_string<Args...> fmt, Args&&... args) {
        draw_text_2d(std::format(fmt, std::forward<Args>(args)...), position, color, scale);
    }


    virtual bool load_font(const std::string& font_path, int point_size, const std::string& font_name = "") = 0;


    /// RESOUCES
    struct TextureInfo {
        Uint32 id;
        int width;
        int height;
    };

    std::unordered_map<std::string, Uint32> _textures;
    std::unordered_map<Uint32, TextureInfo> _texture_info_cache; // texture_id -> dimensions
    std::unordered_map<std::string, std::vector<MeshInstance3D>> _meshes;
    std::unordered_map<std::string, std::vector<Material>> _materials;

    std::shared_ptr<Framebuffer> get_shadow_map_fbo() const;

    std::shared_ptr<Framebuffer> get_main_fbo() const;

protected:
    SDL_Window* _window = nullptr;

    // ========================================================================
    // 3D Rendering Resources
    // ========================================================================
    std::unique_ptr<Shader> _default_shader     = nullptr;
    std::unique_ptr<Shader> _shadow_shader      = nullptr;
    std::unique_ptr<Shader> _environment_shader = nullptr;

    WorldEnvironment* _world_environment = nullptr;

    std::shared_ptr<Framebuffer> shadow_map_fbo = nullptr;
    std::shared_ptr<Framebuffer> main_fbo       = nullptr;

    int _virtual_width = 0, _virtual_height = 0;

    std::shared_ptr<GpuBuffer> instance_buffer;

    std::unordered_map<MeshMaterialKey, RenderBatch, MeshMaterialKeyHash> _instanced_batches;

    std::unordered_map<const MeshInstance3D*, RenderBatch> _shadow_batches;

    // ========================================================================
    // 2D Rendering Resources
    // ========================================================================
    std::unique_ptr<Shader> _shader_2d;
    std::shared_ptr<GpuBuffer> _vertex_buffer_2d;
    std::shared_ptr<GpuBuffer> _index_buffer_2d;
    std::shared_ptr<GpuVertexLayout> _vertex_layout_2d;

    std::vector<DrawCommand2D> _draw_commands_2d;
    std::vector<Vertex2D> _batch_vertices_2d;
    std::vector<uint32_t> _batch_indices_2d;


    std::unordered_map<std::string, TTF_Font*> _fonts;
    TTF_Font* _default_font = nullptr; /// For regular text (auto-set on load)
    TTF_Font* _emoji_font   = nullptr; /// For emojis - auto-detected by name (Twemoji, emoji, etc.)


    struct CachedTextTexture {
        Uint32 texture_id;
        int width;
        int height;
    };

    std::unordered_map<std::string, CachedTextTexture> _cached_text_textures;

    std::shared_ptr<Framebuffer> _2d_framebuffer = nullptr;


};
