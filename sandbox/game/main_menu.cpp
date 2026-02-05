#include "main_menu.h"

#include "core/application.h"
#include "core/input/cursor.h"
#include "scene/game_object.h"
#include "scene/scene.h"

void MainMenu::Awake() {
    spdlog::info("MainMenu::Awake");
    
    golias::Cursor::SetCursorLockState(golias::ECursorLockState::CURSOR_UNLOCKED);
    golias::Cursor::SetCursorEnabled(true);
}

void MainMenu::Start() {
    // MenuController is now a child of MenuCanvas, so parent is the canvas
    auto* canvas = GetParent();
    if (!canvas) {
        spdlog::error("MainMenu::Start - No parent canvas!");
        return;
    }

    // Find and bind Play button
    if (auto* playButtonObj = canvas->FindChildByName("PlayButton")) {
        playButton = playButtonObj->GetComponent<golias::ButtonWidgetComponent>();
        if (playButton) {
            playButton->OnButtonClick = [this]() { OnPlayClicked(); };
        }
    }

    // Find and bind Quit button  
    if (auto* quitButtonObj = canvas->FindChildByName("QuitButton")) {
        quitButton = quitButtonObj->GetComponent<golias::ButtonWidgetComponent>();
        if (quitButton) {
            quitButton->OnButtonClick = [this]() { OnQuitClicked(); };
        }
    }
}

void MainMenu::Update(float deltaTime) {
    // Menu logic can go here (animations, etc.)
}

void MainMenu::OnPlayClicked() {
    spdlog::info("MainMenu: Play clicked - Starting game...");
    
    // Clean transition to game scene
    golias::Scene::ChangeTo("scenes/main.gscene");
}

void MainMenu::OnQuitClicked() {
    spdlog::info("MainMenu: Quit clicked - Exiting...");
    
    if (auto* app = golias::Engine::GetInstance().GetApplication()) {
        app->Close();
    }
}
