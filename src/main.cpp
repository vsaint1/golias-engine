#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;


Texture tex1;
Texture tex2;
Font mine_font;
SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL,SDL_WINDOW_METAL)) {
        return SDL_APP_FAILURE;
    }

    SetTargetFPS(60);

    tex1      = LoadTexture("sprites/Character_001.png");
    tex2      = LoadTexture("sprites/Character_002.png");
    mine_font = LoadFont("fonts/Minecraft.ttf", 32);

    return SDL_APP_CONTINUE;
}


float angle = 0;

SDL_AppResult SDL_AppIterate(void* app_state) {

    ClearBackground({120, 100, 100, 255});
    BeginDrawing();

    DrawTexture(tex1, {20, 50, tex1.width, tex1.height});


    DrawText(mine_font, "WHATS UP my friend \ntest \nit works? \nidk, i think so! \n No emojis =(", {100,400}, {255, 255, 255, 255});


    char msg[256];
    SDL_snprintf(msg, sizeof(msg), "Debug Angle: %.2f", angle);
    
    DrawText(mine_font, msg, {100,600}, {255, 0, 0, 255});

    DrawTexture(tex2, {600, 0, tex1.width, tex1.height});

    angle += 1.0f;

    if (angle > 360.0f) {
        angle = 0.0f;
    }

    // TODO: we can get the scale/origin dynamically
    DrawTextureEx(tex1, {192, 160, 32, 32}, {800, 350, 128, 128}, {64, 64}, 0.0f);

    DrawTextureEx(tex1, {64, 32, 32, 32}, {900, 350, 128, 128}, {64, 64}, 0.0f);

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
    UnloadTexture(mine_font.texture);

    CloseWindow();
}
