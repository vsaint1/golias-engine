#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

Texture tex1;
Texture tex2;
Texture tex3;

Font error_font;
Font default_font;
std::vector<Texture> textures;

Transform text_transform;
Transform text_transform2;
Transform text_transform3;

Camera2D camera;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    SetTargetFPS(60);

    tex1 = LoadTexture("sprites/Character_001.png");
    tex2 = LoadTexture("sprites/Character_002.png");
    tex3 = LoadTexture("sprites/Tools.png");

    // for (int i = 0; i < 32; i++) {
    //     auto tex = LoadTexture("sprites/Character_002.png");
    //     textures.push_back(tex);
    // }

    default_font = LoadFont("fonts/Minecraft.ttf", 32);

    error_font = LoadFont("fonts/test_unk.ttf", 32);

    text_transform.position = glm::vec3(10.0f, 10.f, 0.f);
    text_transform.rotation = glm::vec3(0.f);
    text_transform.scale    = glm::vec3(1.f);

    text_transform2.position = glm::vec3(550.f, 50.f, 0.f);
    text_transform2.rotation = glm::vec3(0.f);
    text_transform2.scale    = glm::vec3(1.f);

    text_transform3.position = glm::vec3(10.f, 350.f, 0.f);
    text_transform3.rotation = glm::vec3(0.f);
    text_transform3.scale    = glm::vec3(1.f);

    camera.transform.position = glm::vec3(100.f, 50.f, 0.0f);
    camera.transform.rotation = glm::vec3(0.f);
    camera.transform.scale    = glm::vec3(0.5f);

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

    DrawText(error_font, "This shouldnt draw", text_transform, {255, 0, 0, 255});

    BeginMode2D(camera);

    DrawText(default_font,
             "Hello World \ntest \nit works? \nidk, i think so! \nNo emojis =( \nInternationalization (i18n) or UTF-8 "
             "is working? \nNão",
             text_transform2, {255, 255, 255, 255});

    EndMode2D();

    DrawTexture(tex3, {20, 50, tex1.width, tex1.height});

    DrawLine({100, 600}, {800, 600}, {255, 0, 0, 255}, 100);

    char msg[256];
    SDL_snprintf(msg, sizeof(msg), "Angle: %.2f", angle);
    DrawText(default_font, msg, text_transform3, {255, 0, 0, 255}, 5.f); // this will get clamped

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

    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        GetRenderer()->OpenGL.viewport[0] = event->window.data1;
        GetRenderer()->OpenGL.viewport[1] = event->window.data2;
        glViewport(0, 0, event->window.data1, event->window.data2);
    }

    auto pKey = SDL_GetKeyboardState(0);
    
    // TODO: add delta time
    if (pKey[SDL_SCANCODE_D]) {
        camera.transform.position.x += 10.0f;
    }

    if (pKey[SDL_SCANCODE_A]) {
        camera.transform.position.x -= 10.0f;
    }

    if (pKey[SDL_SCANCODE_W]) {
        camera.transform.position.y -= 10.0f;
    }

    if (pKey[SDL_SCANCODE_S]) {
        camera.transform.position.y += 10.0f;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {

    UnloadTexture(tex1);
    UnloadTexture(tex2);
    UnloadTexture(tex3);

    UnloadTexture(default_font.texture);

    for (auto& tex : textures) {
        UnloadTexture(tex);
    }

    CloseWindow();
}
