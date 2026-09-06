#include "game.h"

#include "hurt_platform.h"
#include "medkit.h"
#include "player.h"
#include "test_obj.h"

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
    mGodette        = mRoot->FindChildByName("Godette");

    if (GameObject* playbutton = mCanvas->FindChildByName("PlayButton")) {
        ButtonComponent* button = playbutton->GetComponent<ButtonComponent>();
        button->onClick         = [this]() {
            if (mRoot && !mRoot->IsActive()) {
                mRoot->SetActive(true);
                mCanvas->SetActive(false);
                Engine::GetInstance().GetInputManager().SetCanvasFocus(false);
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
        slider->onValueChanged       = [](float value) { Engine::GetInstance().GetAudioManager().SetMasterVolume(value); };
    }

    if (GameObject* settingsVsync = mSettingsCanvas->FindChildByName("VsyncToggle")) {
        CheckBoxComponent* checkBox = settingsVsync->GetComponent<CheckBoxComponent>();
        checkBox->onValueChanged    = [](bool value) { GOLIAS_LOG_INFO("VSync toggled: %d", value); };
    }

    if (GameObject* exitbutton = mCanvas->FindChildByName("QuitButton")) {
        ButtonComponent* button = exitbutton->GetComponent<ButtonComponent>();
        button->onClick         = []() { Engine::GetInstance().Quit(); };
    }

    if (GameObject* hudCanvas = mRoot->FindChildByName("HUDCanvas")) {
        mHUDCanvas = hudCanvas;
    }

    if (GameObject* hudAnimationDropdown = mHUDCanvas->FindChildByName("HUDAnimation_DD")) {
        DropdownComponent* dropdown = hudAnimationDropdown->GetComponent<DropdownComponent>();
        dropdown->ClearOptions();

        if (mGodette) {
            if (AnimationComponent* anim = mGodette->GetComponent<AnimationComponent>()) {
                for (const auto& [name, clip] : anim->GetAnimationClips()) {
                    dropdown->AddOption(name);
                }

                anim->Play(dropdown->GetSelectedOption(), false);
            }
        }

        dropdown->onValueChanged = [dropdown, this](int index) {
            if (mGodette) {
                if (AnimationComponent* anim = mGodette->GetComponent<AnimationComponent>()) {
                    String selectedOption = dropdown->GetSelectedOption();
                    anim->Play(selectedOption, false);
                }
            }
        };
    }


    return true;
}

void GameApplication::Update(float deltaTime) {

    Engine& engine             = Engine::GetInstance();
    InputManager& inputManager = engine.GetInputManager();


    if (mRoot && mRoot->IsActive() && inputManager.IsKeyJustPressed(KeyCode::LeftControl)) {


        if (inputManager.IsCanvasFocused()) {
            inputManager.SetCanvasFocus(false);
        } else {
            inputManager.SetCanvasFocus(true);
        }
    }

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {

        if (mRoot && mRoot->IsActive()) {
            mRoot->SetActive(false);
            inputManager.SetCanvasFocus(true);
            mCanvas->SetActive(true);
        }

        if (mSettingsCanvas && mSettingsCanvas->IsActive()) {
            mSettingsCanvas->SetActive(false);
            mCanvas->SetActive(true);
        }
    }

    if (mHUDCanvas && mHUDCanvas->IsActive()) {

        if (TextComponent* text = mHUDCanvas->FindChildByName("HUDText_Stats")->GetComponent<TextComponent>()) {
            RenderStats rs = engine.GetRenderStats();
            MemoryStats ms = engine.GetMemoryStats();
            String fmt     = String_Format("FPS: %d\nFrame Time: %.2f ms\nCPU: %.2f ms\nGPU: %.2f ms\nRendering\nDraw Calls: %d\nBatches: "
                                           "%d\nVertices: %d\nTriangles: %d\nRAM: %.2f/%s \nVRAM: N/A",
                                           rs.Fps,
                                           rs.FrameTimeMs,
                                           rs.CpuTimeMs,
                                           rs.GpuTimeMs,
                                           rs.DrawCalls,
                                           rs.Batches,
                                           rs.Vertices,
                                           rs.Triangles,
                                           ms.ProcessRamBytes / pow(1024.0, 2),
                                           String_FormatBytes(ms.TotalRamBytes).c_str());
            text->SetText(fmt);
        }
    }

    engine.GetScene()->Update(deltaTime);
}

void GameApplication::Shutdown() {
}
