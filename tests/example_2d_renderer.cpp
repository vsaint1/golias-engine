// Example of using the 2D Renderer with draw commands
// This demonstrates how to use the begin_frame_2d/end_frame_2d pipeline

#include "core/engine.h"

void example_2d_rendering() {
    auto* renderer = GEngine->get_renderer();
    
    // Set virtual resolution (e.g., 320x180 for retro look)
    // This will render at 320x180 and stretch to fit the actual window
    // The image will be scaled to whatever the window size is
    static bool virtual_res_set = false;
    if (!virtual_res_set) {
        // renderer->set_2d_virtual_resolution(800, 600);  // Low-res retro style
        // renderer->set_2d_virtual_resolution(640, 360);  // Medium res
        // renderer->set_2d_virtual_resolution(1920, 1080);  // HD
        renderer->set_2d_virtual_resolution(0, 0);  // Disable (use native window size)
        virtual_res_set = true;
    }

    static Camera2D camera;

    auto window_size = GEngine->get_config().get_window();

    // IMPORTANT: Set camera viewport to match virtual resolution (or window size if disabled)
    camera.viewport_size = glm::vec2(window_size.width, window_size.height);  // Match the virtual resolution
    camera.position = glm::vec2(0, 0);
    camera.zoom = 1.0f;

    static Transform2D camera_transform;

    // Begin 2D rendering frame
    renderer->begin_frame_2d();

    // Draw a filled rectangle
    renderer->draw_rect_2d(
        glm::vec2(100, 100),   // position
        glm::vec2(200, 150),   // size
        glm::vec4(1, 0, 0, 1), // red color
        true                   // filled
    );

    renderer->draw_text_2d(
        "Hello World!",
        glm::vec2(300, 20),
        glm::vec4(1, 0, 0, 1),
        1.0f
    );

    renderer->draw_text_2d(
        "Olá mundo",
        glm::vec2(50, 10),
        glm::vec4(1, 1, 1, 1),
        1.0f
    );

    renderer->draw_text_2d(
        "Hello russian, мир! 🎮",
        glm::vec2(110, 120),
        glm::vec4(1, 1, 1, 1),
        1.0f
    );

    // Emoji rendering with Twemoji font
    renderer->draw_text_2d(
        "😀🎮🚀💎✨",
        glm::vec2(20, 60),
        glm::vec4(1, 1, 1, 1),
        1.0f
    );

    static int frame_count = 0;
    frame_count++;
    renderer->draw_text_2d_fmt(
        glm::vec2(500, 10),
        glm::vec4(0, 0, 1, 1),
        1.0f,
        "Frame: {} | FPS: {:.1f}",
        frame_count,
        1.0 / GEngine->get_timer().delta
    );

    renderer->draw_rect_2d(
        glm::vec2(350, 100),   // position
        glm::vec2(200, 150),   // size
        glm::vec4(0, 1, 0, 1), // green color
        false                  // outlined
    );

    // Draw a line
    renderer->draw_line_2d(
        glm::vec2(100, 300),   // start
        glm::vec2(500, 400),   // end
        glm::vec4(0, 0, 1, 1), // blue color
        3.0f                   // thickness
    );

    // Draw a filled circle
    renderer->draw_circle_2d(
        glm::vec2(700, 200),   // center
        75.0f,                 // radius
        glm::vec4(1, 1, 0, 1), // yellow color
        true,                  // filled
        32                     // segments
    );

    // Draw a circle outline
    renderer->draw_circle_outline_2d(
        glm::vec2(900, 200),   // center
        75.0f,                 // radius
        glm::vec4(1, 0, 1, 1), // magenta color
        5.0f,                  // thickness
        32                     // segments
    );

    // Draw a filled triangle
    renderer->draw_triangle_2d(
        glm::vec2(100, 500),   // point 1
        glm::vec2(200, 500),   // point 2
        glm::vec2(150, 400),   // point 3
        glm::vec4(0, 1, 1, 1), // cyan color
        true                   // filled
    );

    // Draw an outlined triangle
    renderer->draw_triangle_2d(
        glm::vec2(250, 500),   // point 1
        glm::vec2(350, 500),   // point 2
        glm::vec2(300, 400),   // point 3
        glm::vec4(1, 0.5, 0, 1), // orange color
        false                  // outlined
    );

    for (int i = 0; i < 100; ++i) {
       renderer->draw_triangle_2d(
           glm::vec2(400 + i * 30, 500),   // point 1
           glm::vec2(430 + i * 30, 500),   // point 2
           glm::vec2(415 + i * 30, 400),   // point 3
           glm::vec4(0.1f * i, 0.2f * i, 0.3f * i, 1), // gradient color
           true                   // filled
       );
    }

    Uint32 gd_tex = renderer->load_texture_from_file("res/icon.png");
    renderer->draw_texture_2d(
        gd_tex,
        glm::vec2(600, 400),
        glm::vec2(128, 128)
    );
    
    renderer->end_frame_2d(camera, camera_transform);
}
