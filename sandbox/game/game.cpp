#include "game.h"


std::shared_ptr<golias::Material> mat = nullptr;

void SpawnBox(const glm::vec3& position) {

    auto boxExtents = glm::vec3(1.f);
    auto mesh       = golias::Mesh::CreateBox(boxExtents);

    auto obj = golias::Engine::GetInstance().GetScene()->CreateObject("Box");

    obj->SetPosition(position);

    obj->AddComponent(new golias::MeshComponent(mesh, mat));

    auto collider = std::make_shared<golias::BoxCollider>(boxExtents);
    auto body     = std::make_shared<golias::RigidBody>(golias::EBodyType::DYNAMIC, collider, 50.0f);

    // body->SetFriction(0.6f);
    // body->SetRestitution(0.0f);
    // body->SetDragging(0.1f);
    // body->SetAngularDragging(0.05f);

    obj->AddComponent(new golias::PhysicsComponent(body));
}


void SandboxApplication::RegisterTypes() {
    Player::Register();
}

bool SandboxApplication::Initialize() {

    std::srand((unsigned) std::time(nullptr));

#if defined(SCENE_LOAD_FROM_FILE)

    auto Tscene = golias::Scene::Load("scenes/main.gscene");
    auto material = golias::Material::Load("materials/brick.mat");
    mat = material;
    // auto avocado = golias::GameObject::LoadModel("models/avocado/Avocado.gltf",Tscene.get());
    // avocado->SetPosition({-2.0f, 2.0f, -2.0f});
    // avocado->SetScale({25.0f, 25.0f, 25.0f});

    golias::Engine::GetInstance().SetScene(Tscene);
#else
    auto& fs = golias::Engine::GetInstance().GetFileSystem();

    auto rd = golias::Engine::GetInstance().GetRenderingDevice();


    auto mesh = golias::Mesh::CreateBox();

    auto scene = std::make_shared<golias::Scene>();
    golias::Engine::GetInstance().SetScene(scene);

    auto player = scene->CreateObject<Player>("Player");
    player->Start();

    auto monkey = golias::GameObject::LoadModel("models/suzanne/Suzanne.gltf", scene.get());
    monkey->SetPosition({0.0f, 2.0f, -2.0f});

    auto avocado = golias::GameObject::LoadModel("models/avocado/Avocado.gltf", scene.get());
    avocado->SetPosition({-2.0f, 2.0f, -2.0f});
    avocado->SetScale({25.0f, 25.0f, 25.0f});

    auto torus = golias::GameObject::LoadModel("models/torus.obj", scene.get());
    torus->SetPosition({5.0f, 2.0f, -2.0f});

    mat = material;

    auto godette = golias::GameObject::LoadModel("models/godette/godette.gltf", scene.get());
    godette->SetPosition({-5.0f, 2.0f, -6.0f});

    auto nagonford = golias::GameObject::LoadModel("models/nagonford/Nagonford_Animated.glb", scene.get());
    nagonford->SetPosition({0.0f, 2.0f, -6.0f});

    auto ground = scene->CreateObject("Ground");
    ground->SetPosition({0.0f, 0.0f, -1.0f});

    auto groundExtents = glm::vec3(50.0f, 2.0f, 50.0f);
    auto groundMesh    = golias::Mesh::CreateBox(groundExtents);
    ground->AddComponent(new golias::MeshComponent(groundMesh, material));

    auto groundCollider = std::make_shared<golias::BoxCollider>(groundExtents);

    auto groundBody = std::make_shared<golias::RigidBody>(golias::EBodyType::STATIC, groundCollider);

    ground->AddComponent(new golias::PhysicsComponent(groundBody));


#endif

    spdlog::info("GameApplication Initialized successfully.");
    return true;
}

void SandboxApplication::Update(float deltaTime) {
    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsKeyJustPressed(SDLK_E)) {

        float x = (std::rand() % 10 - 5);
        float z = (std::rand() % 10 - 5);

        SpawnBox({x, 8.0f, z});

        spdlog::info("Spawned box at {}, {}, {}", x, 8.0f, z);
    }


    if (auto scene = golias::Engine::GetInstance().GetScene()) {
        scene->Update(deltaTime);
    }
}

void SandboxApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
