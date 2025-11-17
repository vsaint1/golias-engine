#include "core/component/comp_system.h"

#include "core/engine.h"

// =============================================================================
// 2D Rendering Systems
// =============================================================================
void render_rectangles_2d_system() {
    auto* renderer = GEngine->get_renderer();

    GEngine->get_world().each([&](const Rectangle2D& rect, const Transform2D& transform) {
        renderer->draw_rect_2d(transform.position, rect.size, rect.color, rect.filled);
    });
}

void render_circles_2d_system() {
    auto* renderer = GEngine->get_renderer();

    GEngine->get_world().each([&](const Circle2D& circle, const Transform2D& transform) {
        if (circle.filled) {
            renderer->draw_circle_2d(transform.position, circle.radius, circle.color, true, circle.segments);
        } else {
            renderer->draw_circle_outline_2d(transform.position, circle.radius, circle.color, circle.thickness, circle.segments);
        }
    });
}

void render_lines_2d_system() {
    auto* renderer = GEngine->get_renderer();

    GEngine->get_world().each([&](const Line2D& line, const Transform2D& transform) {
        glm::vec2 end = transform.position + line.end_point;
        renderer->draw_line_2d(transform.position, end, line.color, line.thickness);
    });
}

void render_triangles_2d_system() {
    auto* renderer = GEngine->get_renderer();

    GEngine->get_world().each([&](const Triangle2D& triangle, const Transform2D& transform) {
        glm::vec2 p1 = transform.position;
        glm::vec2 p2 = transform.position + triangle.point2;
        glm::vec2 p3 = transform.position + triangle.point3;
        renderer->draw_triangle_2d(p1, p2, p3, triangle.color, triangle.filled);
    });
}

void render_labels_2d_system() {
    auto* renderer = GEngine->get_renderer();

    GEngine->get_world().each([&](const Label2D& label, const Transform2D& transform) {
        renderer->draw_text_2d(label.text, transform.position, label.color, label.scale);
    });
}

void render_sprites_2d_system() {
    auto* renderer = GEngine->get_renderer();

    GEngine->get_world().each([&](const Sprite2D& sprite, const Transform2D& transform) {
        std::shared_ptr<GpuTexture> gpu_texture = nullptr;

        if (renderer->_textures.contains(sprite.texture_name)) {
            gpu_texture = renderer->_textures[sprite.texture_name];
        } else {

            renderer->load_texture_from_file(sprite.texture_name);
            if (renderer->_textures.contains(sprite.texture_name)) {
                gpu_texture = renderer->_textures[sprite.texture_name];
            }
        }

        if (!gpu_texture) {
            spdlog::warn("Failed to load texture for sprite: {}", sprite.texture_name);
            return;
        }

        glm::vec2 size = sprite.size;


        if (size.x == 0 || size.y == 0) {
            size = glm::vec2(gpu_texture->get_width(), gpu_texture->get_height());
        }

        renderer->draw_texture_2d(gpu_texture, transform.position, size, {}, FlipMode::NONE, sprite.color);
    });
}

void render_world_2d_system() {
    auto* renderer = GEngine->get_renderer();

    static Camera2D camera;
    static Transform2D camera_transform;

    GEngine->get_world().each([&](const Camera2D& cam, const Transform2D& transform) {
        camera           = cam;
        camera_transform = transform;
    });

    float v_width  = static_cast<float>(renderer->get_virtual_width());
    float v_height = static_cast<float>(renderer->get_virtual_height());

    camera.viewport_size = glm::vec2(v_width, v_height);

    renderer->begin_frame_2d();

    render_rectangles_2d_system();
    render_circles_2d_system();
    render_lines_2d_system();
    render_triangles_2d_system();
    render_sprites_2d_system();
    render_labels_2d_system();

    renderer->end_frame_2d(camera, camera_transform);
}

glm::mat4 calculate_light_system(const glm::vec3& camer_pos, const glm::mat4& camera_view, const glm::mat4& camera_proj,
                                 std::vector<DirectionalLight3D>& directional_lights) {

    glm::mat4 light_space_matrix(1.0f);

    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, DirectionalLight3D& light) {
        directional_lights.push_back(light);

        if (light.castShadows && light_space_matrix == glm::mat4(1.0f)) {

            light_space_matrix = light.get_light_space_matrix(camer_pos);

            // Alternative: Frustum-fitted shadow map
            // light_space_matrix = light.get_light_space_matrix(camera_view,camera_proj);
        }
    });

    return light_space_matrix;
}

// TODO: spotlight shadows???
void calculate_spot_light_system(std::vector<std::pair<Transform3D, SpotLight3D>>& spot_lights) {
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, SpotLight3D& light) { spot_lights.emplace_back(t, light); });
}

void render_world_3d_system() {

    static Camera3D main_camera;
    static Transform3D camera_transform;
    std::vector<DirectionalLight3D> directionalLights;
    std::vector<std::pair<Transform3D, SpotLight3D>> spotLights;

    GEngine->get_world().each([&](flecs::entity e, const Camera3D& cam, const Transform3D& transform) {
        if (!e.has<tags::MainCamera>()) {
            return;
        }

        main_camera      = cam;
        camera_transform = transform;
    });

    calculate_spot_light_system(spotLights);

    const auto renderer = GEngine->get_renderer();

    renderer->begin_frame();
    const auto query = GEngine->get_world().query<const Transform3D, const MeshRef, const MaterialRef>();

    query.each([&](const Transform3D& transform, const MeshRef& mesh, const MaterialRef& material) {
        renderer->add_to_render_batch(transform, mesh, material);
        renderer->add_to_shadow_batch(transform, mesh);
    });

    const glm::mat4 main_camera_view = main_camera.get_view(camera_transform);
    const glm::mat4 main_camera_proj = main_camera.get_projection(renderer->get_virtual_width(), renderer->get_virtual_height());

    const glm::mat4 light_space_matrix =
        calculate_light_system(camera_transform.position, main_camera_view, main_camera_proj, directionalLights);


    // renderer->begin_environment_pass();
    // renderer->end_environment_pass();

    renderer->begin_shadow_pass();
    renderer->render_shadow_pass(light_space_matrix);
    renderer->end_shadow_pass();

    renderer->begin_render_target();
    renderer->render_main_target(main_camera, camera_transform, light_space_matrix, directionalLights, spotLights);
    renderer->end_render_target();
}
