#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;


Texture tex1;
Texture tex2;
Font error_font;
Font default_font;
std::vector<Texture> textures;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    SetTargetFPS(60);

    tex1 = LoadTexture("sprites/Character_001.png");
    tex2 = LoadTexture("sprites/Character_002.png");

    for (int i = 0; i < 32; i++) {
        auto tex = LoadTexture("sprites/Character_002.png");
        textures.push_back(tex);
    }

    default_font = LoadFont("fonts/Minecraft.ttf", 32);

    error_font = LoadFont("fonts/test_unk.ttf", 32);


    return SDL_APP_CONTINUE;
}


float angle = 0;

SDL_AppResult SDL_AppIterate(void* app_state) {

    angle += 1.0f;

    if (angle > 360.0f) {
        angle = 0.0f;
    }

    ClearBackground({120, 100, 100, 255});
    BeginDrawing();


    DrawText(error_font,"This shouldnt work", {10, 10}, {255, 0, 0, 255});
    
    DrawText(default_font, "Hello World \ntest \nit works? \nidk, i think so! \nNo emojis =(", {550, 50},
             {255, 255, 255, 255});

    DrawTexture(tex1, {20, 50, tex1.width, tex1.height});

    char msg[256];
    SDL_snprintf(msg, sizeof(msg), "Angle: %.2f", angle);

    DrawText(default_font, msg, {10, 350}, {255, 0, 0, 255}, 5.f); // this will get clamped

    DrawLine({100, 600}, {800, 600}, {255, 0, 0, 255}, 20);

    DrawTextureEx(tex2, {0, 0, 32, 32}, {500, 350, 128, 128}, {64, 64}, angle);

    EndDrawing();

    // TODO: just for saving resources
    SDL_Delay(16);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {
    
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if(event->type == SDL_EVENT_WINDOW_RESIZED){
        GetRenderer()->OpenGL.viewport[0] = event->window.data1;
        GetRenderer()->OpenGL.viewport[1] = event->window.data2;

        glViewport(0, 0, event->window.data1, event->window.data2);
    }

    auto pKey = SDL_GetKeyboardState(0);

    if (pKey[SDL_SCANCODE_E]) {
        angle += 1.0f;
        if (angle > 360.0f) {
            angle = 0.0f;
        }
    }

    if (pKey[SDL_SCANCODE_Q]) {
        angle -= 1.0f;
        if (angle < 0.0f) {
            angle = 360.0f;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {

    UnloadTexture(tex1);
    UnloadTexture(tex2);
    UnloadTexture(default_font.texture);
    for (auto& tex : textures) {
        UnloadTexture(tex);
    }

    CloseWindow();
}
