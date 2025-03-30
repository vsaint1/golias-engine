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

Camera2D camera = Camera2D(480, 270);

Audio *mine_music, *tel_music;


const char* gui_text = R"(
    [CONTROLS] - Desktop/Web
    
    [ESC] - Exit
    [WASD] - Move the camera
    [MOUSE_WHEEL] - Zoom (in/out)

    [NUM_KEYS]
     [1] - Play Lullaby
     [2] - Play The Entertainer

    [CONTROLS] - Mobile
     - Finger motion

    This sample just show how to load textures, 
    fonts, audio and use them.

    Version: 0.0.7
    )";


SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    if (!InitAudio()) {
        return SDL_APP_FAILURE;
    }

    mine_music = Mix_LoadAudio("sounds/lullaby.mp3");

    tel_music = Mix_LoadAudio("sounds/the_entertainer.ogg");

    if(SystemInfo::IsMobile())
    {
        Mix_PlayAudio(mine_music);
    }

    tex1 = LoadTexture("sprites/Character_001.png");
    tex2 = LoadTexture("sprites/Character_002.png");
    tex3 = LoadTexture("sprites/Tools.png");


    default_font = LoadFont("fonts/Minecraft.ttf", 16);

    error_font = LoadFont("fonts/test_unk.ttf", 32);

    text_transform.position = glm::vec3(10.0f, 50.f, 0.f);
    text_transform.rotation = glm::vec3(0.f);
    text_transform.scale    = glm::vec3(1.f);

    text_transform2.position = glm::vec3(700.f, 400.f, 1.f); // ofscreen for testing
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


    angle += 1.0f;

    if (angle > 360.0f) {
        angle = 0.0f;
    }

    ClearBackground({120, 100, 100, 255});
    // EMBER_TIMER_START();
    BeginDrawing();


       DrawText(error_font, "This shouldnt draw", text_transform, {255, 0, 0, 150});


    BeginMode2D(camera);


    DrawText(default_font,
             "Hello World \ntest \nit works? \nidk, i think so! \nNo emojis =( \nInternationalization (i18n) or UTF-8 "
             "is working? \nNão",
             text_transform2, {255, 255, 255, 255});

    DrawTextureEx(tex2, {0, 0, 32, 32}, {500, 350, 128, 128}, {64, 64}, angle);

    DrawTexture(tex3, {0, 0, 256, 256});

    EndMode2D();


    BeginCanvas();

    char platform_text[256];
    SDL_snprintf(platform_text, sizeof(platform_text), "Platform: %s, Battery Percentage: %d",
                 SystemInfo::GetPlatform().c_str(), SystemInfo::GetBatteryPercentage());

    DrawText(default_font, platform_text, text_transform, {125, 0, 0, 255});

    char fps[256];
    SDL_snprintf(fps, sizeof(fps), "FPS: %.2f, Zoom: %.2f", core.Time->GetFps(), camera.GetZoom());


    DrawText(default_font, gui_text,
             {
                 glm::vec3(20.0f, 100.f, 0.f),
                 glm::vec3(0.f),
                 glm::vec3(1.f),
             },
             {255, 255, 255, 255});

    DrawText(default_font, fps,
             {
                 glm::vec3(20.0f, 410.f, 0.f),
                 glm::vec3(0.f),
                 glm::vec3(1.f),
             },
             {255, 255, 255, 255});


    DrawRectFilled({0, 10, 500, 420}, {0, 0, 0, 50});


    DrawLine({100, 600}, {800, 600}, {255, 0, 0, 255}, 2);

    EndCanvas();


    EndDrawing();
    // EMBER_TIMER_END("Drawing Procedure");

    core.Time->FixedFrameRate(60);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {


    auto pKey = SDL_GetKeyboardState(0);

    if (pKey[SDL_SCANCODE_ESCAPE] || event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (pKey[SDL_SCANCODE_1]) {
        Mix_PauseAudio(tel_music);
        Mix_PlayAudio(mine_music);
    }

    if (pKey[SDL_SCANCODE_2]) {
        Mix_PauseAudio(mine_music);
        Mix_PlayAudio(tel_music);
    }

    if (event->type == SDL_EVENT_FINGER_DOWN || event->type == SDL_EVENT_FINGER_MOTION) {
        float dx = event->tfinger.dx * (float)GetRenderer()->viewport[0];
        float dy = event->tfinger.dy * (float)GetRenderer()->viewport[1];

        camera.transform.position.x -= dx; 
        camera.transform.position.y -= dy;
    }


    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        int bbWidth, bbHeight;
        SDL_GetWindowSizeInPixels(GetRenderer()->window, &bbWidth, &bbHeight);

        core.Resize(event->window.data1, event->window.data2); // window
        GetRenderer()->Resize(bbWidth, bbHeight); // back_buffer
    }

    if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        float zoom = camera.GetZoom() + event->wheel.y * 0.1f;
        camera.SetZoom(zoom);
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
