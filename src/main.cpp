#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

Texture2D tex1;
Texture2D tex2;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

    CreateRenderer(window,glContext,SCREEN_WIDTH, SCREEN_HEIGHT);

    tex1 = LoadTexture("sprites/Character_001.png");
    tex2 = LoadTexture("sprites/Character_002.png");

    return SDL_APP_CONTINUE;
}


float angle = 0;

SDL_AppResult SDL_AppIterate(void* app_state) {

    ClearBackground({120, 100, 100, 255});
    BeginDrawing();

    DrawTexture(tex1, {0, 0, tex1.width, tex1.height});
    
    angle += 1.0f;
    
    if (angle > 360.0f){
        angle = 0.0f;
    }

    DrawTextureEx(tex2, {192, 160, 32,32}, {800, 350, 128, 128}, {16.f,16.f}, angle, {0,0,0,255});

    DrawTexture(tex2, {200, 300, tex1.width, tex1.height});

    EndDrawing();
    
    SDL_Delay(16);
    
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    auto pKey = SDL_GetKeyboardState(0);

    if(pKey[SDL_SCANCODE_E]){
        angle += 1.0f;
        if (angle > 360.0f){
            angle = 0.0f;
        }
    }

    if(pKey[SDL_SCANCODE_Q]){
        angle -= 1.0f;
        if (angle < 0.0f){
            angle = 360.0f;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {

    UnloadTexture(tex1); 
    UnloadTexture(tex2);

    DestroyRenderer();

}
