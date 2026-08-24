#include "game.h"

#include "test_obj.h"


bool GameApplication::Initialize() {

    mScene                           = new Scene();
    CameraComponent* cameraComponent = new CameraComponent(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

    GameObject* cameraObject = mScene->CreateGameObject("Main Camera");
    cameraObject->AddComponent(cameraComponent);
    cameraObject->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));

    cameraObject->AddComponent(new PlayerControllerComponent());

    mScene->SetMainCamera(cameraObject);

    GameObject* keyLight = mScene->CreateGameObject("Key Light");
    keyLight->AddComponent(new LightComponent());
    keyLight->SetRotation(glm::vec3(0.5f, 0.0f, 0.0f));

    mScene->CreateGameObject<TestObject>("Test Object");

    Ref<Mesh> suzanneMesh         = Mesh::Load("models/suzanne/Suzanne.gltf");
    Ref<Material> suzanneMaterial = Material::Load("materials/suzanne.gmat");

    GameObject* suzanneObject = mScene->CreateGameObject("Suzanne");
    suzanneObject->AddComponent(new StaticMeshComponent(suzanneMesh, suzanneMaterial));
    suzanneObject->SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));

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

    Engine::GetInstance().SetScene(mScene);

    return true;
}

void GameApplication::Update(float deltaTime) {


    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    mScene->Update(deltaTime);
}

void GameApplication::Shutdown() {
}
