#include "core/component/comp_system.h"
#include "core/engine.h"


void render_world_2d_system() {
    auto* renderer = GEngine->get_renderer();

    Camera2D camera;
    Transform2D camera_transform;

    GEngine->get_world().each([&](const Camera2D& cam, const Transform2D& transform) {
        camera           = cam;
        camera_transform = transform;
    });

    float v_width  = static_cast<float>(renderer->get_virtual_width());
    float v_height = static_cast<float>(renderer->get_virtual_height());

    camera.viewport_size = glm::vec2(v_width, v_height);

    renderer->begin_frame_2d();

    float margin_x = v_width * 0.05f;
    float margin_y = v_height * 0.05f;

    renderer->draw_rect_2d(glm::vec2(margin_x, margin_y), glm::vec2(v_width * 0.15f, v_height * 0.2f), glm::vec4(1, 0, 0, 1), // red color
                           true);

    renderer->draw_text_2d("Hello World!", glm::vec2(margin_x + v_width * 0.2f, margin_y), glm::vec4(1, 0, 0, 1), 1.0f);

    renderer->draw_text_2d("Olá mundo", glm::vec2(margin_x, margin_y + v_height * 0.25f), glm::vec4(1, 1, 1, 1), 1.0f);

    renderer->draw_text_2d("Hello russian, мир! 🎮", glm::vec2(margin_x + v_width * 0.08f, margin_y + v_height * 0.15f),
                           glm::vec4(1, 1, 1, 1), 1.0f);

    renderer->draw_text_2d("😀🎮🚀💎✨", glm::vec2(margin_x, margin_y + v_height * 0.06f), glm::vec4(1, 1, 1, 1), 1.0f);

    static int frame_count = 0;
    frame_count++;
    renderer->draw_text_2d_fmt(glm::vec2(v_width * 0.5f, margin_y), glm::vec4(0, 0, 1, 1), 1.0f, "Frame: {} | FPS: {:.1f}", frame_count,
                               1.0 / GEngine->get_timer().delta);

    // Draw outlined rectangle (right-center area)
    renderer->draw_rect_2d(glm::vec2(v_width * 0.65f, v_height * 0.3f), glm::vec2(v_width * 0.25f, v_height * 0.2f),
                           glm::vec4(0, 1, 0, 1), // green color
                           false);

    // Draw a line (left side, diagonal)
    renderer->draw_line_2d(glm::vec2(margin_x, v_height * 0.5f), glm::vec2(v_width * 0.35f, v_height * 0.65f),
                           glm::vec4(0, 0, 1, 1), // blue color
                           3.0f);

    // Draw a filled circle (center-left)
    renderer->draw_circle_2d(glm::vec2(v_width * 0.25f, v_height * 0.5f), v_height * 0.1f, glm::vec4(1, 1, 0, 1), // yellow color
                             true, 32);

    // Draw a circle outline (center-right)
    renderer->draw_circle_outline_2d(glm::vec2(v_width * 0.65f, v_height * 0.5f), v_height * 0.1f, glm::vec4(1, 0, 1, 1), // magenta color
                                     5.0f, 32);

    // Draw a filled triangle (bottom-left)
    renderer->draw_triangle_2d(glm::vec2(margin_x, v_height * 0.85f), glm::vec2(margin_x + v_width * 0.1f, v_height * 0.85f),
                               glm::vec2(margin_x + v_width * 0.05f, v_height * 0.7f), glm::vec4(0, 1, 1, 1), // cyan color
                               true);

    // Draw an outlined triangle (bottom-center)
    renderer->draw_triangle_2d(glm::vec2(margin_x + v_width * 0.15f, v_height * 0.85f), glm::vec2(margin_x + v_width * 0.25f, v_height * 0.85f),
                               glm::vec2(margin_x + v_width * 0.2f, v_height * 0.7f), glm::vec4(1, 0.5, 0, 1), // orange color
                               false);

    // Draw gradient triangles (bottom area)
    // int triangle_count = static_cast<int>(vwidth / 30.0f);
    // for (int i = 0; i < triangle_count && i < 100; ++i) {
    //     float x_pos = margin_x + vwidth * 0.3f + i * (vwidth * 0.3f) / triangle_count;
    //     float triangle_size = vwidth * 0.02f;
    //
    //     renderer->draw_triangle_2d(
    //         glm::vec2(x_pos, vheight * 0.85f),
    //         glm::vec2(x_pos + triangle_size, vheight * 0.85f),
    //         glm::vec2(x_pos + triangle_size * 0.5f, vheight * 0.75f),
    //         glm::vec4(0.1f * i / 10.0f, 0.2f * i / 10.0f, 0.3f * i / 10.0f, 1),
    //         true
    //     );
    // }

    // Draw texture (bottom-right)
    static Uint32 gd_tex = renderer->load_texture_from_file("res/icon.png");
    renderer->draw_texture_2d(gd_tex, glm::vec2(v_width * 0.75f, v_height * 0.7f), glm::vec2(v_width * 0.1f, v_height * 0.15f));

    renderer->end_frame_2d(camera, camera_transform);
}


void render_world_3d_system() {

}