#include "game.h"

#include "hurt_platform.h"
#include "player.h"
#include "test_obj.h"

void GameApplication::RegisterTypes() {

    Player::Register();
    TestObject::Register();
    HurtPlatform::Register();
}

bool GameApplication::Initialize() {

    Ref<Scene> scene = Scene::Load("scene/main.gscene");

    scene->PrintTree();

    Engine::GetInstance().SetScene(scene);

    mRoot   = scene->FindGameObjectByName("Main");
    mCanvas = scene->FindGameObjectByName("Canvas");

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

    if (GameObject* exitbutton = mCanvas->FindChildByName("QuitButton")) {
        ButtonComponent* button = exitbutton->GetComponent<ButtonComponent>();
        button->onClick         = [this]() { Engine::GetInstance().Quit(); };
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
    }

    Engine::GetInstance().GetScene()->Update(deltaTime);
}

void GameApplication::Shutdown() {
}
