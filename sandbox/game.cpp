#include "game.h"

#include "hurt_platform.h"
#include "medkit.h"
#include "player.h"

void GameApplication::RegisterTypes() {

    Player::Register();
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
    mProtoCharacter        = scene->Instantiate("scene/prefabs/proto_character.gprefab", mRoot);

    if (mProtoCharacter) {
        mProtoCharacter->SetPosition(glm::vec3(-10.0f, 0.5f, 9.0f));
    }

    if (mRoot) {
        if (GameObject* spinCube = scene->Instantiate("scene/prefabs/spin_cube.gprefab", mRoot)) {
            // Whatever
        }
    }

    if (mRoot) {
        GameObject* softBodyObject = scene->CreateGameObject("SoftBodyPlane", mRoot);
        softBodyObject->SetPosition(glm::vec3(-4.0f, 5.0f, -1.0f));

        Ref<Mesh> plane        = Mesh::CreatePlane(glm::vec2(4.0f, 3.0f), 24, 18);
        Ref<Material> material = Engine::GetInstance().GetAssetManager().Load<Material>("materials/cloth.gmat");
        Ref<SoftBody> softBody = std::make_shared<SoftBody>(4.0f, 3.0f, 24, 18, SoftBodyPinnedCorners::Top);

        softBodyObject->AddComponent(new StaticMeshComponent(plane, material));
        softBodyObject->AddComponent(new SoftBodyComponent(plane, softBody));
    }

    Engine::GetInstance().GetInputManager().RequestCanvasFocus(true);

    if (GameObject* playbutton = mCanvas->FindChildByName("PlayButton")) {
        ButtonComponent* button = playbutton->GetComponent<ButtonComponent>();
        button->onClick         = [this]() {
            if (mRoot && !mRoot->IsActive()) {
                mRoot->SetActive(true);
                mCanvas->SetActive(false);
                Engine::GetInstance().GetInputManager().RequestCanvasFocus(false);
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

    if (GameObject* settingsBack = mSettingsCanvas->FindChildByName("BackButton")) {
        ButtonComponent* button = settingsBack->GetComponent<ButtonComponent>();
        button->onClick         = [this]() {
            if (mSettingsCanvas) {
                mSettingsCanvas->SetActive(false);
                mCanvas->SetActive(true);
                Engine::GetInstance().GetInputManager().RequestCanvasFocus(true);
            }
        };
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

    if (GameObject* hudDebugToggle = mHUDCanvas->FindChildByName("PhysicsCheckBox")) {
        CheckBoxComponent* checkBox = hudDebugToggle->GetComponent<CheckBoxComponent>();
        checkBox->onValueChanged    = [](bool value) {
            Engine::GetInstance().GetPhysicsManager().GetDebugDrawer().SetDebugMode(value ? PhysicsDebugMode::Wireframe
                                                                                          : PhysicsDebugMode::None);
        };
    }

    if (GameObject* hudAnimationDropdown = mHUDCanvas->FindChildByName("HUDAnimation_DD")) {
        DropdownComponent* dropdown = hudAnimationDropdown->GetComponent<DropdownComponent>();
        dropdown->ClearOptions();

        if (mProtoCharacter) {
            if (AnimationComponent* anim = mProtoCharacter->GetComponent<AnimationComponent>()) {
                for (const auto& [name, clip] : anim->GetAnimationClips()) {
                    dropdown->AddOption(name);
                }

                anim->Play(dropdown->GetSelectedOption(), false);
            }
        }

        dropdown->onValueChanged = [dropdown, this](int index) {
            if (mProtoCharacter) {
                if (AnimationComponent* anim = mProtoCharacter->GetComponent<AnimationComponent>()) {
                    String selectedOption = dropdown->GetSelectedOption();
                    anim->Play(selectedOption, true);
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
            inputManager.RequestCanvasFocus(false);
        } else {
            inputManager.RequestCanvasFocus(true);
        }
    }

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {

        if (mRoot && mRoot->IsActive()) {
            mRoot->SetActive(false);
            inputManager.RequestCanvasFocus(true);
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
