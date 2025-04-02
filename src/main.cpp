#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

Font mine_font;

Texture player_texture;
bool bShowMetrics = false;

Color background_color = {120, 100, 100, 255};

Audio *mine_music, *tel_music, *random_music;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("GUI sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    if (!InitAudio()) {
        return SDL_APP_FAILURE;
    }

    mine_font = LoadFont("fonts/Minecraft.ttf", 16);

    player_texture = LoadTexture("sprites/Character_001.png");

    // assets in examples/assets
    mine_music   = Mix_LoadAudio("sounds/Lullaby.mp3");
    tel_music    = Mix_LoadAudio("sounds/the_entertainer.ogg");
    random_music = Mix_LoadAudio("sounds/test.flac");

    LOG_INFO("Device Name %s", SystemInfo::GetDeviceName().c_str());
    LOG_INFO("Device Model %s", SystemInfo::GetDeviceModel().c_str());
    LOG_INFO("Device UniqueIdentifier %s", SystemInfo::GetDeviceUniqueIdentifier().c_str());


    return SDL_APP_CONTINUE;
}


Color text_color = {255, 255, 255, 255};

SDL_AppResult SDL_AppIterate(void* app_state) {

    core.Input->Update();

    core.Time->Update();

    ClearBackground(background_color);
    BeginDrawing();

    static float angle         = 0.0f;
    static glm::ivec3 position = {500.0, 350.0, 0.0};

    static Transform transform = {
        glm::vec3(500.f, 295.f, 0.f),
        glm::vec3(0.f),
        glm::vec3(1.f),
    };
    DrawTextureEx(player_texture, {0, 0, 32, 32}, {position.x, position.y, 128, 128}, {64, 64}, angle);

    DrawText(mine_font, "I guess this works", transform, text_color, 0.0f);
    BeginCanvas();


    ImGui::SetNextWindowSize(ImVec2(350.f, 600.f), ImGuiCond_FirstUseEver);
    ImGui::Begin("[DEMO] - example with GUI", nullptr, ImGuiWindowFlags_NoCollapse);


    float temp_color[4] = {text_color.r / 255.0f, text_color.g / 255.0f, text_color.b / 255.0f, text_color.a / 255.0f};


    ImGui::Text("Player");
    ImGui::SliderInt3("Position##player", &position.x, 0.0f, core.Window.width);
    ImGui::SliderFloat("Angle##player", &angle, 0.0f, 360.0f, "%.2f");

    ImGui::Text("Text");
    ImGui::SliderFloat3("Position##text", &transform.position.x, 0.0f, core.Window.width);
    ImGui::SliderFloat3("Scale##text", &transform.scale.x, 0.0f, 10.0f);
    ImGui::SliderFloat3("Rotation##text", &transform.rotation.x, 0.0f, 360.0f);

    if (ImGui::ColorEdit4("Text color", temp_color), ImGuiColorEditFlags_NoInputs) {
        text_color.r = static_cast<uint8_t>(temp_color[0] * 255);
        text_color.g = static_cast<uint8_t>(temp_color[1] * 255);
        text_color.b = static_cast<uint8_t>(temp_color[2] * 255);
        text_color.a = static_cast<uint8_t>(temp_color[3] * 255);
    }


    ImGui::Text("Musics");

    if (!Mix_IsAudioPlaying(mine_music)) {

        if (ImGui::Button("Play Lullaby")) {
            Mix_PlayAudio(mine_music);
        }

    } else {
        if (ImGui::Button("Stop Lullaby")) {
            Mix_PauseAudio(mine_music);
        }
    }

    if (ImGui::SliderFloat("Volume##1", &mine_music->volume, 0.0f, 1.0f)) {
        Mix_SetVolume(mine_music, mine_music->volume);
    }

    if (!Mix_IsAudioPlaying(tel_music)) {

        if (ImGui::Button("Play The Entertainer")) {
            Mix_PlayAudio(tel_music);
        }

    } else {
        if (ImGui::Button("Stop The Entertainer")) {

            Mix_PauseAudio(tel_music);
        }
    }

    if (ImGui::SliderFloat("Volume##2", &tel_music->volume, 0.0f, 1.0f)) {
        Mix_SetVolume(tel_music, tel_music->volume);
    }

    if (!Mix_IsAudioPlaying(random_music)) {

        if (ImGui::Button("Play Unknown")) {
            Mix_PlayAudio(random_music);
        }

    } else {
        if (ImGui::Button("Stop The Unknown")) {

            Mix_PauseAudio(random_music);
        }
    }

    if (ImGui::SliderFloat("Volume##3", &random_music->volume, 0.0f, 1.0f)) {
        Mix_SetVolume(random_music, random_music->volume);
    }


    ImGui::Text("Engine");
    if (ImGui::SliderFloat("Musics Volume", &core.Audio.global_volume, 0.0f, 100.0f)) {
        Mix_SetGlobalVolume(core.Audio.global_volume);
    }

    ImGui::Checkbox("Metrics", &bShowMetrics);

    ImGui::End();


    if (bShowMetrics) {
        ImGui::Begin("Metrics", &bShowMetrics);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Application average: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Delta Time: %f", core.Time->GetDeltaTime());
        ImGui::Text("Elapsed Time: %.3f", core.Time->GetElapsedTime());
        ImGui::Text("Frame count: %d", core.Time->GetFrameCount());

        ImGui::Text("RAM Usage: %d MB", GetMemoryUsage());


        if (ImGui::Button("Close Me")) {
            bShowMetrics = false;
        }

        ImGui::End();
    }


    EndCanvas();


    EndDrawing();


    core.Time->FixedFrameRate(60);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {

    core.Input->ProcessEvents(*event);

    auto pKey = SDL_GetKeyboardState(0);

    if (pKey[SDL_SCANCODE_ESCAPE] || event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }


    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        core.Resize(event->window.data1, event->window.data2);

        GetRenderer()->Resize(event->window.data1, event->window.data2);
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {

    UnloadTexture(mine_font.texture);
    UnloadTexture(player_texture);

    CloseAudio();

    CloseWindow();
}
