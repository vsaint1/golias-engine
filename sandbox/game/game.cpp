#include "game.h"

#include "scene/2d/sprite_component_2d.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/text_component.h"

std::shared_ptr<golias::Material> mat = nullptr;

void SpawnBox(const glm::vec3& position) {

    if (!mat) {
        return;
    }

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

    auto Tscene   = golias::Scene::Load("scenes/main.gscene");
    auto material = golias::Material::Load("materials/brick.mat");
    mat           = material;
    auto avocado = golias::Model::Load("models/avocado/Avocado.gltf",Tscene.get());
    avocado->SetPosition({-2.0f, 2.0f, -2.0f});
    avocado->SetScale({25.0f, 25.0f, 25.0f});

    // auto Tscene = std::make_shared<golias::Scene>();

    auto spriteObj = Tscene->CreateObject("Sprite");
    spriteObj->SetPosition2D({500.0f, 500.0f});
    spriteObj->SetRotation2D(45.0f);

    auto brickTex = golias::Texture2D::Load("textures/brick.png");

    auto spriteComp = new golias::SpriteComponent2D();
    spriteComp->SetTexture(brickTex);
    spriteComp->SetSize({100.0f, 100.0f});

    spriteObj->AddComponent(spriteComp);

    // auto camera = Tscene->CreateObject("Camera");
    // camera->AddComponent(new golias::CameraComponent());
    // Tscene->SetMainCamera(camera);

    auto canvas     = Tscene->CreateObject("Canvas");
    auto canvasComp = new golias::CanvasComponent();
    canvasComp->SetCanvasMode(golias::ECanvasMode::WORLD_SPACE);
    canvas->AddComponent(canvasComp);

    auto text     = Tscene->CreateObject("Text", canvas);
    auto textComp = new golias::TextWidgetComponent();
    textComp->SetText("Hello, Golias Engine! こんにちは 안녕하세요 привет");
    // textComp->SetFont("fonts/Minecraft.ttf", 32);
    // textComp->SetShadowEnabled(true);
    textComp->SetOutlineEnabled(true);


    text->AddComponent(textComp);

    text->SetPosition({10.0f, 150.0f, 2.0f});
    // text->SetPosition2D({400.0f, 300.0f});

    auto canvas_2d    = Tscene->CreateObject("Canvas2D");
    auto canvasComp2D = new golias::CanvasComponent();
    canvasComp2D->SetCanvasMode(golias::ECanvasMode::SCREEN_SPACE);
    canvas_2d->AddComponent(canvasComp2D);

    auto text2             = Tscene->CreateObject("Text2D", canvas_2d);
    auto textComp2         = new golias::TextWidgetComponent();
    std::string infoString = R"(First Person Demo
Press 'E' to spawn boxes 
Press 'T' to toggle physics debug draw
WASD to move, Mouse to look around)";

    textComp2->SetText(infoString);
    text2->SetPosition2D({300.0f, 200.0f});
    textComp2->SetShadowEnabled(true);
    text2->AddComponent(textComp2);

    golias::Engine::GetInstance().SetScene(Tscene);
#else
    auto& fs = golias::Engine::GetInstance().GetFileSystem();

    auto rd = golias::Engine::GetInstance().GetRenderingDevice();


    auto mesh = golias::Mesh::CreateBox();

    auto scene = std::make_shared<golias::Scene>();
    golias::Engine::GetInstance().SetScene(scene);

    auto player = scene->CreateObject<Player>("Player");
    player->Start();

    auto monkey = golias::Model::Load("models/suzanne/Suzanne.gltf", scene.get());
    monkey->SetPosition({0.0f, 2.0f, -2.0f});

    auto avocado = golias::Model::Load("models/avocado/Avocado.gltf", scene.get());
    avocado->SetPosition({-2.0f, 2.0f, -2.0f});
    avocado->SetScale({25.0f, 25.0f, 25.0f});

    auto torus = golias::Model::Load("models/torus.obj", scene.get());
    torus->SetPosition({5.0f, 2.0f, -2.0f});

    mat = material;

    auto godette = golias::Model::Load("models/godette/godette.gltf", scene.get());
    godette->SetPosition({-5.0f, 2.0f, -6.0f});

    auto nagonford = golias::Model::Load("models/nagonford/Nagonford_Animated.glb", scene.get());
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

    if (input.IsKeyJustPressed(SDLK_T)) {
        auto& physicsDebugDrawer = golias::Engine::GetInstance().GetPhysicsManager();
        physicsDebugDrawer.SetDebugDrawEnabled(!physicsDebugDrawer.IsDebugDrawEnabled());
        spdlog::info("Toggled Physics Debug Draw: {}", physicsDebugDrawer.IsDebugDrawEnabled() ? "ON" : "OFF");
    }

    if (auto scene = golias::Engine::GetInstance().GetScene()) {
        scene->Update(deltaTime);
    }
}

void SandboxApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
