#include "core/renderer/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

Font mine_font;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {

    if (!InitWindow("Window sample", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    if (!InitAudio()) {
        return SDL_APP_FAILURE;
    }

    mine_font = LoadFont("fonts/Minecraft.ttf", 16);

    LOG_INFO("Device Name %s", SystemInfo::GetDeviceName().c_str());
    LOG_INFO("Device Model %s", SystemInfo::GetDeviceModel().c_str());
    LOG_INFO("Device UniqueIdentifier %s", SystemInfo::GetDeviceUniqueIdentifier().c_str());


    return SDL_APP_CONTINUE;
}


std::string text_hold = "";

SDL_AppResult SDL_AppIterate(void* app_state) {

    core.Time->Update();

    core.Input->Update();

    ClearBackground({120, 100, 100, 255});

    BeginDrawing();

// TODO: create a canvas component InputText
#pragma region TEXT_INPUT

    ember::Rectangle input_rect = {300, 300, 100, 300};

    DrawText(mine_font, "Feedback",
             {
                 glm::vec3(300.f, 295.f, 0.f),
                 glm::vec3(0.f),
                 glm::vec3(1.f),
             },
             {255, 255, 255, 255}, 0.0f);

    DrawRectFilled(input_rect, {0, 0, 0, 100});

    if (core.Input->IsPositionInRect(core.Input->GetMousePosition(), input_rect)
        && core.Input->IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        core.Input->SetTextInputActive(true);
    }

    else if (!core.Input->IsPositionInRect(core.Input->GetMousePosition(), input_rect)
             && core.Input->IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        core.Input->SetTextInputActive(false);
    }

    if (core.Input->IsTextInputActive()) {
        DrawRect({input_rect.x, input_rect.y, input_rect.width, input_rect.height}, {255, 255, 255, 100}, 4.0f);

        text_hold = core.Input->GetTypedText();

        std::vector<std::string> lines;
        std::string current_line;
        size_t max_characters_per_line = input_rect.width / (mine_font.font_size * 0.8);

        for (char ch : text_hold) {

            if (ch == '\n' || current_line.size() >= max_characters_per_line) {
                lines.push_back(current_line);
                current_line.clear();
            }

            if (ch != '\n') {
                current_line += ch;
            }
        }

        if (!current_line.empty()) {
            lines.push_back(current_line);
        }

        size_t max_lines = input_rect.height / mine_font.font_size;

        if (lines.size() > max_lines) {
            lines.resize(max_lines);
        }

        if (lines.size() == max_lines && text_hold.length() > max_characters_per_line * max_lines) {
            lines[lines.size() - 1] = lines[lines.size() - 1].substr(0, max_characters_per_line) + "...";
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            std::string visible_text = lines[i];

            DrawText(
                mine_font, visible_text.c_str(),
                {
                    glm::vec3(input_rect.x + 5.f, input_rect.y + mine_font.font_size + i * mine_font.font_size, 1.f),
                    glm::vec3(0.f),
                    glm::vec3(1.f),
                },
                {255, 255, 255, 255}, 0.0f);
        }
    }


#pragma endregion

    DrawText(mine_font, TextFormat("Mouse Position  %s", glm::to_string(core.Input->GetMousePosition()).c_str()),
             {
                 glm::vec3(20.0f, 100.f, 0.f),
                 glm::vec3(0.f),
                 glm::vec3(1.f),
             },
             {255, 255, 255, 255}, 0.0f);


    DrawText(mine_font,
             TextFormat("Mouse Pressed? %s", core.Input->IsMouseButtonPressed(SDL_BUTTON_LEFT) ? "TRUE" : "FALSE"),
             {
                 glm::vec3(20.0f, 150.f, 0.f),
                 glm::vec3(0.f),
                 glm::vec3(2.f),
             },
             {255, 255, 255, 255}, 1.0f);

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

    CloseAudio();

    CloseWindow();
}
