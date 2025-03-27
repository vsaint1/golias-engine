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

Camera2D camera = Camera2D(SCREEN_WIDTH, SCREEN_HEIGHT);

Audio *mine_music, *tel_music;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    if (!InitAudio()) {
        return SDL_APP_FAILURE;
    }

    mine_music = Mix_LoadAudio("sounds/lullaby.mp3");

    tel_music = Mix_LoadAudio("sounds/test.flac");

    Mix_PlayAudio(mine_music);

    // Mix_SetVolume(mine_music, 0.1f);

  

    // Mix_PlayAudio(tel_music);

    tex1 = LoadTexture("sprites/Character_001.png");
    tex2 = LoadTexture("sprites/Character_002.png");
    tex3 = LoadTexture("sprites/Tools.png");


    default_font = LoadFont("fonts/Minecraft.ttf", 32);

    error_font = LoadFont("fonts/test_unk.ttf", 32);

    text_transform.position = glm::vec3(20.0f, 50.f, 0.f);
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

    return SDL_APP_CONTINUE;
}


float angle = 0;

int texture_count = 1'000'000;

SDL_AppResult SDL_AppIterate(void* app_state) {

    core.Time->Update();

    angle += 1.0f;

    if (angle > 360.0f) {
        angle = 0.0f;
    }

    ClearBackground({120, 100, 100, 255});
    // EMBER_TIMER_START();
    BeginDrawing();


    DrawText(error_font, "This shouldnt draw", text_transform, {255, 0, 0, 255});


    DrawText(default_font, "Press [W] [A] [S] [D] to move and Mouse Wheel to scale (1 milion textures occluded)",
             text_transform, {125, 0, 0, 255});

    char fps[256];
    SDL_snprintf(fps, sizeof(fps), "FPS: %.2f", core.Time->GetFps());
    DrawText(default_font, fps,
             {
                 glm::vec3(20.0f, 200.f, 0.f),
                 glm::vec3(0.f),
                 glm::vec3(1.f),
             },
             {255, 255, 255, 255});

    BeginMode2D(camera);
    DrawLine({100, 600}, {800, 600}, {255, 0, 0, 255}, 2);

    DrawText(default_font,
             "Hello World \ntest \nit works? \nidk, i think so! \nNo emojis =( \nInternationalization (i18n) or UTF-8 "
             "is working? \nNão",
             text_transform2, {255, 255, 255, 255});


    char msg[256];


    for (int i = 0; i < texture_count; i++) {
        if (camera.IsVisible({i * 32, 350, 0})) {

            DrawTextureEx(tex2, {0, 0, 32, 32}, {i * 32, 350, 128, 128}, {64, 64}, angle);
        }
    }

    SDL_snprintf(msg, sizeof(msg), "Texture count: %d", texture_count);

    DrawTexture(tex3, {0, 0, tex3.width, tex3.height});

    EndMode2D();

    DrawText(default_font, msg, text_transform3, {255, 0, 0, 255}, 5.f);


    EndDrawing();
    // EMBER_TIMER_END("Drawing Procedure");

    core.Time->FixedFrameRate(60);

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
        camera.Resize(bbWidth, bbHeight);
    }

    auto pKey = SDL_GetKeyboardState(0);

    if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        camera.zoom += event->wheel.y * 0.1f;
    }

    // TODO: add delta time
    if (pKey[SDL_SCANCODE_D]) {
        camera.transform.position.x += 10.0f;
        Mix_PauseAudio(mine_music);
    }

    if (pKey[SDL_SCANCODE_A]) {
        camera.transform.position.x -= 10.0f;
        Mix_PlayAudio(mine_music);
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
