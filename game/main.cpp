#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;

struct GameContext {
    Node2D* root;
    RigidBody2D* player;
    Renderer* renderer;
    Label* colliding;
};

GameContext ctx;
SDL_Event e;

const float MOVE_SPEED = 200.0f; // px/s
const float JUMP_FORCE = 10.0f; // px impulse


void game_update() {
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT) {
            GEngine->is_running = false;
        }
        GEngine->input_manager()->process_event(e);
    }

    const bool* state = SDL_GetKeyboardState(nullptr);
    const double dt   = GEngine->time_manager()->get_delta_time();

    glm::vec2 vel = ctx.player->get_velocity();
    if (state[SDL_SCANCODE_A]) {
        vel.x -= MOVE_SPEED * dt;
    }
    if (state[SDL_SCANCODE_D]) {
        vel.x += MOVE_SPEED * dt;
    }
    ctx.player->set_velocity(vel);

    if (state[SDL_SCANCODE_SPACE] && ctx.player->is_on_ground()) {
        ctx.player->apply_impulse({0.0f, JUMP_FORCE});
    }

    // --- Engine update ---
    GEngine->update(dt);
    ctx.root->process(dt);

    // --- Rendering ---
    ctx.renderer->clear({0.2f, 0.3f, 0.3f, 1.0f});
    ctx.root->draw(ctx.renderer);
    ctx.renderer->flush();

    auto win_size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);
    ImGui::Begin("Engine Runtime", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
                     | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {

        // --- File Menu ---
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) { /* TODO: create new scene */
            }
            if (ImGui::MenuItem("Open Scene")) { /* TODO: open file dialog */
            }
            if (ImGui::MenuItem("Save Scene")) { /* TODO: save current scene */
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                GEngine->is_running = false;
            }
            ImGui::EndMenu();
        }

        // --- Project Menu ---
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Settings")) { /* TODO: open project settings */
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show Debug")) { /* toggle debug overlays */
            }
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                ImGui::OpenPopup("AboutPopup");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Ember Engine Example\nVersion 1.2.0\nAuthor: vsantos1");
        ImGui::Text("Source Code");
        ImGui::TextLink("https://github.com/vsaint1/ember_engine");
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    float inspector_width = 300.0f;
    float content_height  = 200.0f;
    float right_width     = win_size.x - inspector_width;
    float viewport_height = win_size.y - content_height - ImGui::GetFrameHeightWithSpacing();

    ImGui::BeginChild("HierarchyPane", ImVec2(inspector_width, viewport_height), true);
    ctx.root->draw_hierarchy();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ViewportInspectorSplit", ImVec2(right_width, viewport_height), false);

    float viewport_width = right_width * 0.7f;
    ImGui::BeginChild("ViewportPane", ImVec2(viewport_width, viewport_height), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("Viewport - (%.1f FPS)", ImGui::GetIO().Framerate);

    ImGui::Spacing();

    const std::string button_txt = GEngine->time_manager()->is_paused() ? "Play" : "Stop";
    if (ImGui::Button(button_txt.c_str(), ImVec2(60, 25))) {
        if (GEngine->time_manager()->is_paused()) {
            GEngine->time_manager()->resume();
        } else {
            GEngine->time_manager()->pause();
        }
    }

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::GetWindowDrawList()->AddRect(ImGui::GetCursorScreenPos(),
                                        ImVec2(ImGui::GetCursorScreenPos().x + avail.x, ImGui::GetCursorScreenPos().y + avail.y),
                                        IM_COL32(255, 255, 255, 255));
    ImGui::Image(ctx.renderer->get_framebuffer_texture(), avail, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::EndChild();

    ImGui::SameLine();


    ImGui::BeginChild("InspectorPane", ImVec2(right_width - viewport_width, viewport_height), true);

    if (g_selected_node) {
        g_selected_node->draw_inspector();
    } else {
        ImGui::Text("No node was selected.");
    }

    ImGui::EndChild();


    ImGui::EndChild();

    ImGui::BeginChild("ContentPane", ImVec2(win_size.x, content_height), true);
    ImGui::Text("Content Browser");
    ImGui::Text("Not implemented yet.");
    ImGui::EndChild();

    ImGui::End();

    ctx.renderer->present();
}

// -------------------- Main --------------------
int main(int argc, char* argv[]) {
    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    const auto renderer = GEngine->get_renderer();
    ctx.renderer        = renderer;

    if (!renderer->load_font("fonts/Minecraft.ttf", "mine", 16)) {
        return SDL_APP_FAILURE;
    }

    // ---- Scene setup ----
    auto sample_texture2 = renderer->load_texture("sprites/Character_002.png");

    Node2D* root = new Node2D("Root");
    ctx.root     = root;

    RigidBody2D* ground = new RigidBody2D();
    ground->body_size   = glm::vec2(320, 20.0f);
    ground->set_transform({glm::vec2(160, 160), glm::vec2(1.f), 0.0f});
    root->add_child("Ground", ground);

    RigidBody2D* player = new RigidBody2D();
    player->body_type   = BodyType::DYNAMIC;
    player->body_size   = {16, 16};
    player->shape_type  = ShapeType::CIRCLE;
    player->radius      = 8.0f;
    player->set_transform({{50, 50}, {1.f, 1.f}, 0.0f});
    ctx.player = player;

    Sprite2D* player_sprite = new Sprite2D(sample_texture2);
    player_sprite->set_region({0, 0, 32, 32}, {32, 32});
    player_sprite->set_z_index(10);
    player->add_child("Sprite", player_sprite);
    root->add_child("Player", player);

    Label* colliding = new Label("mine", "colliding");
    colliding->set_text("Colliding with: None");
    colliding->set_transform({{10, 20}, {1.f, 1.f}, 0.0f});
    ctx.colliding = colliding;
    root->add_child("CollidingTxt", colliding);

    root->ready();

    player->on_body_entered([&](const Node2D* other) {
        if (other) {
            if (colliding) {
                colliding->set_text("Colliding with: %s", other->get_name().c_str());
            }
        }
    });
    player->on_body_exited([&](const Node2D* _) {
        if (colliding) {
            colliding->set_text("Colliding with: None");
        }
    });

    // ---- Main loop ----
    while (GEngine->is_running) {
        game_update();
    }

    // ---- Cleanup ----
    delete root;
    GEngine->shutdown();

    return 0;
}
