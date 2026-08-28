#include "game.h"

#include "player.h"
#include "test_obj.h"
#include <random>

void GameApplication::RegisterTypes() {

    Player::Register();
    TestObject::Register();
}

bool GameApplication::Initialize() {

    Ref<Scene> scene = Scene::Load("scene/main.gscene");

    scene->PrintTree();
    
    Engine::GetInstance().SetScene(scene);


    // GameObject* redLight  = mScene->CreateGameObject("Red Light");
    // LightComponent* lightComp = new LightComponent();
    // lightComp->SetType(LightType::Point);
    // lightComp->SetRange(20.0f);
    // lightComp->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
    // redLight->AddComponent(lightComp);
    // redLight->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));

    // GameObject* greenLight = mScene->CreateGameObject("Green Light");
    // LightComponent* greenLightComp = new LightComponent();
    // greenLightComp->SetType(LightType::Point);
    // greenLightComp->SetColor(glm::vec3(0.0f, 1.0f, 0.0f));
    // greenLight->AddComponent(greenLightComp);
    // greenLight->SetPosition(glm::vec3(2.0f, 2.0f, 0.0f));


    return true;
}

void GameApplication::Update(float deltaTime) {


    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    Engine::GetInstance().GetScene()->Update(deltaTime);
}

void GameApplication::Shutdown() {
}
