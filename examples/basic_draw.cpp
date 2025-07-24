#define SDL_MAIN_USE_CALLBACKS
#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

Font mine_font, sans_font;

Texture player_texture, p2_texture;
bool bShowMetrics = false;

Color background_color = {120, 100, 100, 255};

Audio *mine_music, *tel_music, *random_music;
Camera2D camera = Camera2D(480, 270);

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {
    if (!GEngine->initialize("Example - new API", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    camera.transform.position = glm::vec2(0.f, 0.f);
    camera.transform.rotation = 0.0f;

    // assets in examples/assets
    mine_font = GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", 32);
    sans_font = GEngine->get_renderer()->load_font("fonts/OpenSans.ttf", 24);

    player_texture = GEngine->get_renderer()->load_texture("sprites/Character_001.png");
    p2_texture     = GEngine->get_renderer()->load_texture("sprites/Character_002.png");
    mine_music     = Audio::load("sounds/lullaby.mp3");
    tel_music      = Audio::load("sounds/the_entertainer.ogg");
    random_music   = Audio::load("sounds/test.flac");

    mine_music->play();

    LOG_INFO("Device Name %s", SystemInfo::get_device_name().c_str());
    LOG_INFO("Device Model %s", SystemInfo::get_device_model().c_str());
    LOG_INFO("Device UniqueIdentifier %s", SystemInfo::get_device_unique_identifier().c_str());

    return SDL_APP_CONTINUE;
}

const char* gui_text = R"(
Lorem Ipsum is simply dummy text of the printing and typesetting industry.
Lorem Ipsum has been the industry's standard dummy text ever since the 1500s,
when an unknown printer took a galley of type and scrambled it to make a type specimen book.
It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.
It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages,
and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.

)";

Color text_color = {255, 255, 255, 255};

int entities = 0;

SDL_AppResult SDL_AppIterate(void* app_state) {
    GEngine->input_manager()->update();

    GEngine->time_manager()->update();

    GEngine->get_renderer()->clear_background(background_color);
    GEngine->get_renderer()->begin_drawing();

    static float angle        = 0.0f;
    static glm::vec3 position = {200, 300, 0};

    static Transform transform = {glm::vec2(300.f, 100.f), glm::vec2(1.f), 0.0f};

    static Transform transform2;
    transform2.position = {500, 100};
    transform2.rotation = 0.0f;
    transform2.scale    = {1, 1};

    static Transform transform3 = {glm::vec3(300.f, 450.f, 0.f), glm::vec2(1.f), 0.0f};

    for (int i = 0; i < entities; i++) {
        GEngine->get_renderer()->draw_texture_ex(player_texture, {0, 0, 32, 32}, {i * 64, i * 64, 64, 64}, glm::vec2(0.5f, 0.5f), angle);
    }

    GEngine->get_renderer()->draw_triangle({100, 100, 0.3f}, {200, 100, 0}, {150, 200, 0}, {255, 255, 0, 255});

    GEngine->get_renderer()->draw_line({200, 200, 0.1}, {300, 300, 0}, {0, 255, 0, 255}, 2.f);

    GEngine->get_renderer()->draw_texture(player_texture, transform2, {0, 0});

    GEngine->get_renderer()->draw_rect(transform2, {512, 512}, {255, 255, 255, 255}, 2.f);

    GEngine->get_renderer()->draw_circle({300, 650, 0.2}, 40, {255, 255, 255, 255});
    GEngine->get_renderer()->draw_circle_filled({300, 300, 0.3}, 20, {255, 0, 255, 255});

    GEngine->get_renderer()->draw_texture_ex(p2_texture, {0, 0, 32, 32}, {100, 200, 128, 128}, glm::vec2(0.5f, 0.5f), angle);

    GEngine->get_renderer()->draw_text(sans_font, "I think this works\n No internationalization =(", transform, {255, 255, 255, 255}, 20.f);

    GEngine->get_renderer()->draw_text(mine_font, "Hello World!", transform3, {255, 255, 255, 255}, 20.f,
                                       {.Outline = {
                                            .enabled  = true,
                                            .color     = Color(255, 0, 0, 255).normalize_color(),
                                            .thickness = 0.35f,
                                        }});

    ImGui::SetNextWindowSize(ImVec2(350.f, 600.f), ImGuiCond_FirstUseEver);
    ImGui::Begin("[DEMO] - new API", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::InputInt("Create Entity", &entities, 10);

    float temp_color[4] = {text_color.r / 255.0f, text_color.g / 255.0f, text_color.b / 255.0f, text_color.a / 255.0f};

    ImGui::Text("Player");
    ImGui::DragFloat3("Position##player", &position.x, 1);
    ImGui::SliderFloat("Angle##player", &angle, 0.0f, 360.0f, "%.2f");

    ImGui::Text("Text");
    ImGui::SliderFloat3("Position##text", &transform2.position.x, 0.0f, GEngine->Window.width);
    ImGui::SliderFloat3("Scale##text", &transform2.scale.x, 0.0f, 10.0f);
    ImGui::SliderFloat3("Rotation##text", &transform2.rotation, 0.0f, 360.0f);

    if (ImGui::ColorEdit4("Text color", temp_color), ImGuiColorEditFlags_NoInputs) {
        text_color.r = static_cast<uint8_t>(temp_color[0] * 255);
        text_color.g = static_cast<uint8_t>(temp_color[1] * 255);
        text_color.b = static_cast<uint8_t>(temp_color[2] * 255);
        text_color.a = static_cast<uint8_t>(temp_color[3] * 255);
    }

    ImGui::Text("Camera");
    ImGui::DragFloat3("Position##Camera", &camera.transform.position.x, 1.0f);
    ImGui::DragFloat("Zoom##Camera", &camera.zoom, 0.1f, 0.01f, 10.0f);

    ImGui::Text("Musics");

    if (!mine_music->is_playing()) {
        if (ImGui::Button("Play Lullaby")) {
            mine_music->play();
        }

    } else {
        if (ImGui::Button("Stop Lullaby")) {
            mine_music->pause();
        }
    }

    if (ImGui::SliderFloat("Volume##1", &mine_music->volume, 0.0f, 1.0f)) {
        mine_music->set_volume(mine_music->volume);
    }

    if (!tel_music->is_playing()) {
        if (ImGui::Button("Play The Entertainer")) {
            tel_music->play();
        }

    } else {
        if (ImGui::Button("Stop The Entertainer")) {
            tel_music->pause();
        }
    }

    if (ImGui::SliderFloat("Volume##2", &tel_music->volume, 0.0f, 1.0f)) {
        tel_music->set_volume(tel_music->volume);
    }

    if (!random_music->is_playing()) {
        if (ImGui::Button("Play Unknown")) {
            random_music->play();
        }

    } else {
        if (ImGui::Button("Stop The Unknown")) {
            random_music->pause();
        }
    }

    if (ImGui::SliderFloat("Volume##3", &random_music->volume, 0.0f, 1.0f)) {
        random_music->set_volume(random_music->volume);
    }

    ImGui::Text("Engine");
    if (ImGui::SliderFloat("Musics Volume", &GEngine->Audio.global_volume, 0.0f, 1.0f)) {
        Audio_SetMasterVolume(GEngine->Audio.global_volume);
    }

    ImGui::Checkbox("Metrics", &bShowMetrics);

    ImGui::End();

    if (bShowMetrics) {
        ImGui::Begin("Metrics", &bShowMetrics);

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Application average: %2.03f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Delta Time: %f", GEngine->time_manager()->get_delta_time());
        ImGui::Text("Elapsed Time: %.3f", GEngine->time_manager()->get_elapsed_time());
        ImGui::Text("Frame count: %lu", GEngine->time_manager()->get_frame_count());

        ImGui::Text("RAM Usage: %d MB", get_memory_usage());

        if (ImGui::Button("Close Me")) {
            bShowMetrics = false;
        }

        ImGui::End();
    }

    GEngine->get_renderer()->end_drawing();

    GEngine->time_manager()->fixed_frame_rate();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {
    GEngine->input_manager()->process(event);

    auto pKey = SDL_GetKeyboardState(nullptr);

    if (pKey[SDL_SCANCODE_ESCAPE] || event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        GEngine->resize_window(event->window.data1, event->window.data2);

        GEngine->get_renderer()->resize(event->window.data1, event->window.data2);
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {
    GEngine->get_renderer()->unload_font(mine_font);

    GEngine->get_renderer()->unload_texture(player_texture);

    GEngine->shutdown();
}
