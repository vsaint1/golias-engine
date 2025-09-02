#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;


int main(int argc, char* argv[]) {
    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    auto renderer = GEngine->get_renderer();

    Node2D* root = new Node2D("Root");
    root->set_transform({{10,20}, {1.f, 1.f}, 0.0f});

    SDL_Event e;
    while (GEngine->is_running) {
        while (SDL_PollEvent(&e)) {

            GEngine->input_manager()->process_event(e);
        }

        const double dt = GEngine->time_manager()->get_delta_time();

        GEngine->update(dt);
        root->ready();
        root->process(dt);

        renderer->clear({0.2f, 0.3f, 0.3f, 1.0f});

        root->draw(renderer);
        root->input(GEngine->input_manager());

        renderer->flush();
        renderer->present();
    }

    delete root;
    GEngine->shutdown();

    return 0;
}
