#include "core/systems/editor_manager.h"

#include "core/ember_core.h"
#include "core/engine.h"


void draw_main_menu() {
#if defined(WITH_EDITOR)
    if (ImGui::BeginMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) { /* TODO */ }
            if (ImGui::MenuItem("Open Scene")) { /* TODO */ }
            if (ImGui::MenuItem("Save Scene")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { GEngine->is_running = false; }
            ImGui::EndMenu();
        }

        // Project Menu
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Settings")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Test")) { /* TODO */ }
            ImGui::EndMenu();
        }

        // Help Menu
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) { ImGui::OpenPopup("AboutPopup"); }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Ember Engine Example\nVersion 1.2.0\nAuthor: vsantos1");
        ImGui::Text("Source Code");
        ImGui::TextLink("https://github.com/vsaint1/ember_engine");
        if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
#endif

}

// --- Hierarchy ---
void draw_hierarchy(Node2D* root, float height) {
#if defined(WITH_EDITOR)

    ImGui::BeginChild("HierarchyPane", ImVec2(250, height), true);
    root->draw_hierarchy();
    ImGui::EndChild();
#endif
}

// --- Viewport ---
void draw_viewport(float width, float height) {
#if defined(WITH_EDITOR)

    ImGui::BeginChild("ViewportPane", ImVec2(width, height), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginTable("ViewportToolbar", 5, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextColumn();

        // Play / Stop
        const std::string button_txt = GEngine->time_manager()->is_paused() ? "Play" : "Stop";
        if (ImGui::Button(button_txt.c_str(), ImVec2(60, 25))) {
            if (GEngine->time_manager()->is_paused()) {
                GEngine->time_manager()->resume();
            } else {
                GEngine->time_manager()->pause();
            }
        }

        ImGui::TableNextColumn();
        if (ImGui::Button("TODO", ImVec2(60, 25))) {}

        ImGui::TableNextColumn();
        static bool show_debug = true;
        ImGui::Checkbox("Show Debug", &show_debug);

        ImGui::EndTable();
    }

    ImGui::Separator();

    // Framebuffer Preview
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float target_aspect = 16.0f / 9.0f;
    float current_aspect = avail.x / avail.y;
    ImVec2 viewport_size = avail;

    if (current_aspect > target_aspect) viewport_size.x = avail.y * target_aspect;
    else viewport_size.y = avail.x / target_aspect;

    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 centered_pos(
        cursor_pos.x + (avail.x - viewport_size.x) * 0.5f,
        cursor_pos.y + (avail.y - viewport_size.y) * 0.5f
    );

    ImGui::SetCursorScreenPos(centered_pos);
    ImGui::Image(GEngine->get_renderer()->get_framebuffer_texture(),
                 viewport_size, ImVec2(0, 1), ImVec2(1, 0));

    // Overlay info
    // ImGui::SetCursorScreenPos(centered_pos);
    // ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // draw_list->AddText(ImVec2(centered_pos.x + 10, centered_pos.y + 10),
    //                    IM_COL32(255, 255, 0, 255),
    //                    std::format("FPS: {:.1f}", ImGui::GetIO().Framerate).c_str());

    if (ImGui::IsWindowHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        glm::vec2 local(
            (mouse_pos.x - centered_pos.x) / viewport_size.x,
            1.0f - (mouse_pos.y - centered_pos.y) / viewport_size.y
        );
        // glm::vec2 world = ctx.camera->screen_to_world(local);
    }

    ImGui::EndChild();
#endif
}

// --- Inspector ---
void draw_inspector(Node2D* selected_node, float width, float height) {
#if defined(WITH_EDITOR)

    ImGui::BeginChild("InspectorPane", ImVec2(width, height), true);
    if (selected_node) selected_node->draw_inspector();
    else ImGui::Text("No node selected.");
    ImGui::EndChild();
#endif
}

// --- Console ---
void draw_console(float width, float height) {
#if defined(WITH_EDITOR)

    ImGui::BeginChild("ConsolePane", ImVec2(width, height), true);
    ImGui::Text("Console Output (logs, warnings, errors)");
    ImGui::Text("TODO: hook up logging system.");
    ImGui::EndChild();
#endif
}

// --- Content Browser ---
void draw_content_browser(float width, float height) {
#if defined(WITH_EDITOR)

    ImGui::BeginChild("ContentPane", ImVec2(width, height), true);
    ImGui::Text("Content Browser");
    ImGui::Text("TODO: asset management here.");
    ImGui::EndChild();
#endif
}


void draw_editor(Node2D* root) {
#if defined(WITH_EDITOR)

    // --- Editor Layout ---
    auto win_size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);
    ImGui::Begin("Engine Runtime", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

    draw_main_menu();

    // Reserve space for bottom panel (console/content)
    float bottom_height = 200.0f;
    ImVec2 avail_full   = ImGui::GetContentRegionAvail();

    // ---------------- Top section (Hierarchy | Viewport | Inspector) ----------------
    ImGui::BeginChild("TopSection", ImVec2(avail_full.x, avail_full.y - bottom_height), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 avail_top = ImGui::GetContentRegionAvail();

    // Left: Hierarchy Tabs
    float left_width = 250.0f;
    ImGui::BeginChild("LeftPane", ImVec2(left_width, avail_top.y), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::BeginTabBar("TopLeftTabs")) {
        if (ImGui::BeginTabItem("Hierarchy")) {
            draw_hierarchy(root,avail_top.y);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene")) {
            ImGui::Text("TODO");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Center: Viewport
    float right_width = 300.0f;
    float center_width = avail_top.x - left_width - right_width;

    ImGui::BeginChild("CenterPane", ImVec2(center_width, avail_top.y), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    draw_viewport(center_width, avail_top.y);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right: Inspector Tabs
    ImGui::BeginChild("RightPane", ImVec2(right_width, avail_top.y), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::BeginTabBar("TopRightTabs")) {
        if (ImGui::BeginTabItem("Inspector")) {
            draw_inspector(g_selected_node,right_width, avail_top.y);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Unknown")) {
            ImGui::Text("TODO");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();

    ImGui::EndChild(); // TopSection

    // ---------------- Bottom section (Console | Content) ----------------
    ImGui::BeginChild("BottomSection", ImVec2(avail_full.x, bottom_height), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginTabBar("BottomTabs")) {
        if (ImGui::BeginTabItem("Console")) {
            draw_console(avail_full.x, bottom_height);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Content")) {
            draw_content_browser(avail_full.x, bottom_height);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();

    ImGui::End();
#endif
}
