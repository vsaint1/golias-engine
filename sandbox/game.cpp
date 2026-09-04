#include "game.h"

#include "hurt_platform.h"
#include "player.h"
#include "test_obj.h"
#include "medkit.h"

void GameApplication::RegisterTypes() {

    Player::Register();
    TestObject::Register();
    HurtPlatform::Register();
    Medkit::Register();
}

// TODO: This can be moved to separate classes for better organization
bool GameApplication::Initialize() {

    Ref<Scene> scene = Scene::Load("scene/main.gscene");

    scene->PrintTree();

    Engine::GetInstance().SetScene(scene);

    mRoot           = scene->FindGameObjectByName("Main");
    mCanvas         = scene->FindGameObjectByName("Canvas");
    mSettingsCanvas = scene->FindGameObjectByName("SettingsCanvas");


    if (GameObject* playbutton = mCanvas->FindChildByName("PlayButton")) {
        ButtonComponent* button = playbutton->GetComponent<ButtonComponent>();
        button->onClick         = [this]() {
            if (mRoot && !mRoot->IsActive()) {
                mRoot->SetActive(true);
                mCanvas->SetActive(false);
                Engine::GetInstance().SetInputMode(InputMode::Disabled);
            }
        };
    }

    if (GameObject* settingsbutton = mCanvas->FindChildByName("SettingsButton")) {
        ButtonComponent* button = settingsbutton->GetComponent<ButtonComponent>();
        button->onClick         = [this]() {
            if (mSettingsCanvas) {
                mCanvas->SetActive(false);
                mSettingsCanvas->SetActive(true);
            }
        };
    }

    if (GameObject* settingsMasterSlider = mSettingsCanvas->FindChildByName("MasterSoundSlider")) {
        InputSliderComponent* slider = settingsMasterSlider->GetComponent<InputSliderComponent>();
        slider->onValueChanged = [](float value) {
            Engine::GetInstance().GetAudioManager().SetMasterVolume(value);
            
        };
    }

    if (GameObject* settingsVsync = mSettingsCanvas->FindChildByName("VsyncToggle")) {
        CheckBoxComponent* checkBox = settingsVsync->GetComponent<CheckBoxComponent>();
        checkBox->onValueChanged = [](bool value) {
            GOLIAS_LOG_INFO("VSync toggled: %d", value);
        };
    }

    if (GameObject* exitbutton = mCanvas->FindChildByName("QuitButton")) {
        ButtonComponent* button = exitbutton->GetComponent<ButtonComponent>();
        button->onClick         = []() { Engine::GetInstance().Quit(); };
    }


    return true;
}

void GameApplication::Update(float deltaTime) {


    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {

        if (mRoot && mRoot->IsActive()) {
            mRoot->SetActive(false);
            Engine::GetInstance().SetInputMode(InputMode::Cursor);
            mCanvas->SetActive(true);
        }

        if (mSettingsCanvas && mSettingsCanvas->IsActive()) {
            mSettingsCanvas->SetActive(false);
            mCanvas->SetActive(true);
        }
    }

    Engine::GetInstance().GetScene()->Update(deltaTime);
}

void GameApplication::Shutdown() {
}
