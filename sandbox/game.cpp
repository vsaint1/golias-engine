#include "game.h"

#include "test_obj.h"


bool GameApplication::Initialize() {

    mScene                           = new Scene();
    CameraComponent* cameraComponent = new CameraComponent(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);

    GameObject* cameraObject = mScene->CreateGameObject("Main Camera");
    cameraObject->AddComponent(cameraComponent);
    cameraObject->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));

    cameraObject->AddComponent(new PlayerControllerComponent());

    mScene->SetMainCamera(cameraObject);

    GameObject* keyLight = mScene->CreateGameObject("Key Light");
    keyLight->AddComponent(new LightComponent());
    keyLight->SetRotation(glm::vec3(0.5f, 0.0f, 0.0f));

    mScene->CreateGameObject<TestObject>("Test Object");

    GameObject* suzanneObject = GameObject::Load("models/suzanne/Suzanne.gltf", mScene);
    suzanneObject->SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));

    GameObject* gun = GameObject::Load("models/gun_carbine/scene.gltf", mScene, "gun_carbine");
    gun->SetParent(cameraObject);
    gun->SetPosition(glm::vec3(0.5f, -0.6f, 0.75f));

    if (AnimationComponent* animComp = gun->GetComponent<AnimationComponent>()) {
        if (GameObject* bullet = gun->FindChildByName("bullet_33")) {
            bullet->SetActive(false);
        }

        if (GameObject* fire = gun->FindChildByName("BOOM_35")) {
            fire->SetActive(false);
        }
    }

    Ref<Material> material = Material::Load("materials/checker.gmat");

    glm::vec3 groundExtents = glm::vec3(30.0f, 1.0f, 30.0f);
    Ref<Mesh> cube          = Mesh::CreateCube(groundExtents);

    GameObject* planeObj = mScene->CreateGameObject("Plane");
    planeObj->AddComponent(new StaticMeshComponent(cube, material));
    planeObj->SetPosition(glm::vec3(0.0f, -3.0f, 0.0f));

    Ref<Collider> boxCollider = std::make_shared<BoxCollider>(groundExtents);
    Ref<RigidBody> rigidBody  = std::make_shared<RigidBody>(RigidBodyType::Static, boxCollider);
    planeObj->AddComponent(new PhysicsComponent(rigidBody));

    glm::vec3 boxSize  = glm::vec3(1.0f, 1.0f, 1.0f);
    Ref<Mesh> cubeMesh = Mesh::CreateCube(boxSize);

    Ref<Collider> fallingBox        = std::make_shared<BoxCollider>(boxSize);
    PhysicsMaterial fallingBoxMaterial = {0.5f, 1.0f, 0.0f};
    Ref<RigidBody> fallingRigidBody = std::make_shared<RigidBody>(RigidBodyType::Dynamic, fallingBox, fallingBoxMaterial);

    GameObject* fallingBoxObj = mScene->CreateGameObject("Falling Box");
    fallingBoxObj->AddComponent(new StaticMeshComponent(cubeMesh, material));
    fallingBoxObj->SetPosition(glm::vec3(-5.0f, 5.0f, 2.0f));
    fallingBoxObj->AddComponent(new PhysicsComponent(fallingRigidBody));


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

    if (inputManager.IsMouseButtonJustPressed(MouseButton::Left)) {
        if (GameObject* gun = mScene->FindGameObjectByName("gun_carbine")) {
            if (AnimationComponent* animComp = gun->GetComponent<AnimationComponent>()) {
                animComp->Play("shoot", false);
            }
        }
    }

    Engine::GetInstance().GetScene()->Update(deltaTime);
}

void GameApplication::Shutdown() {
}
