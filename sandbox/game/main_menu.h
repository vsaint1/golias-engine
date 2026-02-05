#pragma once

#include "core/engine.h"
#include "scene/native_behaviour.h"
#include "scene/scene_manager.h"
#include "scene/ui/button_component.h"
#include "scene/ui/canvas_component.h"

/// @brief Main menu controller - handles button callbacks and scene transitions
class MainMenu : public golias::NativeBehaviour {
    GCLASS(MainMenu)
public:
    MainMenu() = default;
    ~MainMenu() = default;

    void Awake() override;
    void Start() override;
    void Update(float deltaTime) override;

private:
    void OnPlayClicked();
    void OnQuitClicked();

    golias::ButtonWidgetComponent* playButton = nullptr;
    golias::ButtonWidgetComponent* quitButton = nullptr;
};
