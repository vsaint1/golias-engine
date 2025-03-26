#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

Texture tex1;
Texture tex2;
Texture tex3;

Font error_font, default_font;
std::vector<Texture> textures;

Transform text_transform;
Transform text_transform2;
Transform text_transform3;

Camera2D camera;

Music *mine_music, *tel_music;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    if (!InitAudio()) {
        return SDL_APP_FAILURE;
    }
    
    SetTargetFPS(60);

    mine_music = Mix_LoadMusic("sounds/lullaby.mp3");

    tel_music = Mix_LoadMusic("sounds/test.flac");

    Mix_PlayMusic(mine_music);

    // Mix_PlayMusic(tel_music);

    tex1 = LoadTexture("sprites/Character_001.png");
    tex2 = LoadTexture("sprites/Character_002w.png");
    tex3 = LoadTexture("sprites/Tools.png");

    // for (int i = 0; i < 32; i++) {
    //     auto tex = LoadTexture("sprites/Character_002.png");
    //     textures.push_back(tex);
    // }

    default_font = LoadFont("fonts/Minecraft.ttf", 32);

    error_font = LoadFont("fonts/test_unk.ttf", 32);

    text_transform.position = glm::vec3(50.0f, 50.f, 0.f);
    text_transform.rotation = glm::vec3(0.f);
    text_transform.scale    = glm::vec3(1.f);

    text_transform2.position = glm::vec3(1500.f, 500.f, 1.f); // ofscreen for testing
    text_transform2.rotation = glm::vec3(0.f);
    text_transform2.scale    = glm::vec3(1.f);

    text_transform3.position = glm::vec3(10.f, 350.f, 0.f);
    text_transform3.rotation = glm::vec3(0.f);
    text_transform3.scale    = glm::vec3(1.f);

    camera.transform.position = glm::vec3(0.f, 0.f, 0.0f);
    camera.transform.rotation = glm::vec3(0.f);
    camera.transform.scale    = glm::vec3(1.f);


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

    DrawLine({100, 600}, {800, 600}, {255, 0, 0, 255}, 100);

    DrawText(default_font, "Press [W] [A] [S] [D] to move and Mouse Wheel to scale", text_transform, {125, 0, 0, 255});

    BeginMode2D(camera);

    DrawText(default_font,
              "Hello World \ntest \nit works? \nidk, i think so! \nNo emojis =( \nInternationalization (i18n) or UTF-8 "
              "is working? \nNão",
              text_transform2, {255, 255, 255, 255});

    DrawTextureEx(tex2, {0, 0, 32, 32}, {500, 350, 128, 128}, {64, 64}, angle);


    EndMode2D();

    DrawTexture(tex3, {20, 50, tex1.width, tex1.height});


    char msg[256];
    SDL_snprintf(msg, sizeof(msg), "Angle: %.2f", angle);
    DrawText(default_font, msg, text_transform3, {255, 0, 0, 255}, 5.f); // this will get clamped


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
        int bbWidth, bbHeight;
        SDL_GetWindowSizeInPixels(GetRenderer()->window, &bbWidth, &bbHeight);

        glViewport(0, 0, bbWidth, bbHeight);
    }

    auto pKey = SDL_GetKeyboardState(0);

    if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        camera.transform.scale.x += event->wheel.y * 0.1f;
        camera.transform.scale.y += event->wheel.y * 0.1f;
    }

    // TODO: add delta time
    if (pKey[SDL_SCANCODE_D]) {
        camera.transform.position.x += 10.0f;
        Mix_PauseMusic(mine_music);
    }

    if (pKey[SDL_SCANCODE_A]) {
        camera.transform.position.x -= 10.0f;
        Mix_PlayMusic(mine_music);
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

    CloseAudio();

    CloseWindow();
}
