#include "game.h"

#include "scene/2d/sprite_component_2d.h"
#include "scene/3d/directional_light_component.h"
#include "scene/3d/pointlight_component.h"
#include "scene/3d/skeleton_animation_component.h"
#include "scene/3d/spotlight_component.h"
#include "scene/3d/world_environment_component.h"
#include "scene/ui/button_component.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"
#include "scene/ui/text_component.h"

std::shared_ptr<golias::Material> mat = nullptr;
golias::GameObject* porsche           = nullptr;


void SpawnBox(const glm::vec3& position) {

    if (!mat) {
        return;
    }

    auto boxExtents = glm::vec3(1.f);
    auto mesh       = golias::Mesh::CreateBox(boxExtents);

    auto obj = golias::Engine::GetInstance().GetScene()->CreateObject("Box");

    obj->SetPosition(position);

    obj->AddComponent(new golias::MeshRendererComponent(mesh, mat));

    auto collider = std::make_shared<golias::BoxCollider>(boxExtents);
    auto body     = std::make_shared<golias::RigidBody>(golias::EBodyType::DYNAMIC, collider, 50.0f);

    obj->AddComponent(new golias::PhysicsComponent(body));
}


void SandboxApplication::RegisterTypes() {
    // Register custom NativeBehaviour types
    Player::Register();
    MainMenu::Register();
}

// Forward declaration for the game scene setup callback
void SetupGameScene(golias::Scene* scene);

bool SandboxApplication::Initialize() {
    std::srand((unsigned) std::time(nullptr));

    // Register scene transition callback to setup the game scene when loaded
    golias::SceneManager::GetInstance().OnSceneLoaded = [](golias::Scene* scene) {
        if (scene && scene->GetName() == "Main") {
            SetupGameScene(scene);
        } else {
            // Reset game state when going back to menu
            porsche = nullptr;
        }
    };

    // Start with main menu scene
    golias::Scene::ChangeTo("scenes/menu.gscene");

    spdlog::info("SandboxApplication Initialized with Main Menu.");
    return true;
}

// Called when the game scene (main.gscene) is loaded
void SetupGameScene(golias::Scene* Tscene) {
    spdlog::info("Setting up game scene...");

    auto material = golias::Material::Load("materials/brick.mat");
    mat           = material;

    porsche = golias::Model::Load("models/pbr/porsche/scene.gltf", Tscene);
    porsche->SetPosition({10.0f, 1.0f, 0.0f});
    porsche->SetScale({100.0f, 100.0f, 100.0f});

    auto godette = golias::Model::Load("models/godette/godette.glb", Tscene);
    godette->SetPosition({0.0f, 1.0f, -1.0f});


    auto carCanvas     = Tscene->CreateObject("CanvasT");
    auto carCanvasComp = new golias::CanvasComponent();
    carCanvasComp->SetCanvasMode(golias::ECanvasMode::WORLD_SPACE);

    auto textCar     = Tscene->CreateObject("TextT", carCanvas);
    auto textCarComp = new golias::TextWidgetComponent();
    textCarComp->SetText("Porsche");

    textCar->AddComponent(textCarComp);
    textCar->SetPosition({0.0f, 150.0f, 0.0f});
    porsche->AddComponent(carCanvasComp);

    auto worldEnvironment = Tscene->CreateObject("WorldEnvironment");
    // auto textureCubemap   = golias::TextureCubemap::LoadProcedural();
    auto textureCubemap = golias::TextureCubemap::Load("textures/FrozenWaterfall.hdr");

    auto worldEnvComp = new golias::WorldEnvironmentComponent(textureCubemap, golias::EToneMappingMode::TONE_MAPPING_ACES, 1.0f);
    worldEnvComp->SetEnvironmentMode(golias::EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_SKYBOX);
    // auto worldEnvComp = new golias::WorldEnvironmentComponent();
    worldEnvironment->AddComponent(worldEnvComp);
    worldEnvComp->SetExposure(0.3f);
    // worldEnvComp->SetClearColor({0.8f, 0.7f, 0.6f, 1.0f});

    if (godette) {

        if (auto godetteAnim = godette->GetComponent<golias::SkeletonAnimationComponent>()) {
            godetteAnim->Play("Cheer", true);
        }

        auto godetteCanvas = Tscene->CreateObject("GodetteCanvas", godette);

        auto godetteCanvasComp = new golias::CanvasComponent();
        godetteCanvasComp->SetCanvasMode(golias::ECanvasMode::WORLD_SPACE);
        godetteCanvasComp->SetUseBillboarding(true);
        godetteCanvasComp->SetWorldSpaceScale(0.01f);
        godetteCanvas->AddComponent(godetteCanvasComp);


        auto godetteText = Tscene->CreateObject("GodetteText", godetteCanvas);

        auto godetteTextComp = new golias::TextWidgetComponent();
        godetteTextComp->SetText("Godette");
        godetteTextComp->SetFont("fonts/Minecraft.ttf", 32);
        godetteTextComp->SetFontSize(16);
        godetteTextComp->SetShadowEnabled(true);

        godetteText->AddComponent(godetteTextComp);


        godetteCanvas->SetPosition({0.0f, 2.5f, 0.0f});
        godetteText->SetScale({0.01f, 0.01f, 0.01f});
    }

    // for (int i = 0; i < 3; i++){
    //     auto godette = golias::Model::Load("models/godette/godette.glb", Tscene.get());
    //     godette->SetPosition({static_cast<float>(std::rand() % 20 - 10), 1.0f, static_cast<float>(std::rand() % 20 - 10)});
    //     if (auto godetteAnim = godette->GetComponent<golias::SkeletonAnimationComponent>()) {
    //         godetteAnim->Play("Cheer", true);
    //     }
    // }

    // auto sponza = golias::Model::Load("models/pbr/sponza/Sponza.gltf", Tscene.get());
    // sponza->SetPosition({10.0f, 1.0f, 10.0f});

    // auto spriteObj = Tscene->CreateObject("Sprite");
    // spriteObj->SetPosition2D({500.0f, 500.0f});
    // spriteObj->SetRotation2D(45.0f);

    // auto brickTex = golias::Texture2D::Load("textures/brick.png");

    // auto spriteComp = new golias::SpriteComponent2D();
    // spriteComp->SetTexture(brickTex);
    // spriteComp->SetSize({100.0f, 100.0f});

    // spriteObj->AddComponent(spriteComp);

    // auto camera = Tscene->CreateObject("Camera");
    // camera->AddComponent(new golias::CameraComponent());
    // Tscene->SetMainCamera(camera);

    auto canvas     = Tscene->CreateObject("Canvas");
    auto canvasComp = new golias::CanvasComponent();
    canvasComp->SetCanvasMode(golias::ECanvasMode::WORLD_SPACE);
    // canvasComp->SetUseBillboarding(true);
    canvasComp->SetWorldSpaceScale(0.01f);

    canvas->AddComponent(canvasComp);

    auto text     = Tscene->CreateObject("Text", canvas);
    auto textComp = new golias::TextWidgetComponent();
    textComp->SetText("Hello, Golias Engine! こんにちは 안녕하세요 привет");
    textComp->SetOutlineEnabled(true);


    text->AddComponent(textComp);

    canvas->SetPosition({5.0f, 2.0f, -5.0f});

    auto canvas_2d    = Tscene->CreateObject("Canvas2D");
    auto canvasComp2D = new golias::CanvasComponent();
    canvasComp2D->SetCanvasMode(golias::ECanvasMode::SCREEN_SPACE);
    canvas_2d->AddComponent(canvasComp2D);

    auto canvasRt = new golias::RectTransformComponent();
    canvas_2d->AddComponent(canvasRt);

    auto text2             = Tscene->CreateObject("Text2D", canvas_2d);
    auto textComp2         = new golias::TextWidgetComponent();
    std::string infoString = R"(First Person Demo
Press 'E' to spawn boxes 
Press 'T' to toggle physics debug draw
WASD to move, Mouse to look around)";

    textComp2->SetText(infoString);
    textComp2->SetShadowEnabled(true);
    text2->AddComponent(textComp2);
    
    auto text2Rt = new golias::RectTransformComponent();
    text2Rt->SetAnchor({0.0f, 0.0f}); // Bottom-left anchor
    text2Rt->SetPivot({0.0f, 0.0f});  // Pivot at bottom-left of text
    text2->SetPosition({10.0f, 150.0f,0.0f});
    text2->AddComponent(text2Rt);

    auto button     = Tscene->CreateObject("Button2D", canvas_2d);
    auto buttonComp = new golias::ButtonWidgetComponent();
    button->AddComponent(buttonComp);
    buttonComp->OnButtonClick = []() { spdlog::info("Button Clicked!"); };

    auto buttonRt = new golias::RectTransformComponent();
    buttonRt->SetSize({180.0f, 50.0f});
    buttonRt->SetAnchor({0.5f, 0.5f}); 
    buttonRt->SetPivot({0.5f, 0.5f}); 
    button->AddComponent(buttonRt);

    golias::Cursor::SetCursorLockState(golias::ECursorLockState::CURSOR_LOCKED);
    // golias::Cursor::SetCursorEnabled(false);

    auto buttonText     = Tscene->CreateObject("ButtonText", button);
    auto buttonTextComp = new golias::TextWidgetComponent();
    buttonTextComp->SetText("Click Me");
    buttonTextComp->SetTextColor({0.0f, 0.0f, 0.0f, 1.0f});
    buttonText->AddComponent(buttonTextComp);

    auto buttonTextRt = new golias::RectTransformComponent();
    buttonTextRt->SetAnchor({0.5f, 0.5f}); // Center within button
    buttonTextRt->SetPivot({0.5f, 0.5f});  // Center pivot
    buttonText->SetPosition({0.0f, 0.0f, 0.0f}); // No offset
    buttonText->AddComponent(buttonTextRt);

    auto textAmmo     = Tscene->CreateObject("TextAmmo", canvas_2d);
    auto textCompAmmo = new golias::TextWidgetComponent();
    textCompAmmo->SetText("9/10");
    textCompAmmo->SetFont("fonts/Minecraft.ttf", 32);
    textCompAmmo->SetShadowEnabled(true);
    textAmmo->AddComponent(textCompAmmo);

    auto textAmmoRt = new golias::RectTransformComponent();
    textAmmoRt->SetAnchor({1.0f, 0.0f}); // Bottom-right anchor
    textAmmoRt->SetPivot({1.0f, 0.0f});  // Pivot at bottom-right of text
    textAmmo->SetPosition({-10.0f, 10.0f, 0.0f}); // 10px offset from bottom-right
    textAmmo->AddComponent(textAmmoRt);


    auto& canvasInputManager = golias::Engine::GetInstance().GetCanvasInputManager();
    canvasInputManager.SetActive(true);
    canvasInputManager.SetActiveCanvas(canvasComp2D);

    auto directionalLight     = Tscene->CreateObject("DirectionalLight");
    auto directionalLightComp = new golias::DirectionalLightComponent();
    directionalLightComp->SetDirection({0.5f, -1.0f, 0.3f});
    directionalLightComp->SetColor({1.0f, 1.0f, 1.0f});
    directionalLightComp->SetIntensity(5.0f);
    directionalLightComp->SetCastShadows(true);
    directionalLight->AddComponent(directionalLightComp);

    auto spotLight     = Tscene->CreateObject("SpotLight");
    auto spotLightComp = new golias::SpotlightComponent();
    spotLightComp->SetPosition({2.0f, 5.0f, 2.0f});
    spotLightComp->SetDirection({-0.5f, -1.0f, -0.5f});
    spotLightComp->SetColor({0.0f, 0.0f, 1.0f});
    spotLightComp->SetIntensity(10.0f);
    spotLightComp->SetRange(15.0f);
    spotLightComp->SetInnerConeAngle(15.0f);
    spotLightComp->SetOuterConeAngle(25.0f);
    spotLight->AddComponent(spotLightComp);

    auto pointLight     = Tscene->CreateObject("PointLight");
    auto pointLightComp = new golias::PointLightComponent();
    pointLightComp->SetPosition({-2.0f, 3.0f, -2.0f});
    pointLightComp->SetColor({1.0f, 0.0f, 0.0f});
    pointLightComp->SetIntensity(10.0f);
    pointLight->AddComponent(pointLightComp);

    auto torus = golias::Model::Load("models/torus.obj", Tscene);
    torus->SetPosition({5.0f, 2.0f, -2.0f});

    // Lock cursor for FPS controls
    golias::Cursor::SetCursorLockState(golias::ECursorLockState::CURSOR_LOCKED);

    spdlog::info("Game scene setup complete.");
}

void SandboxApplication::Update(float deltaTime) {
    auto& input = golias::Engine::GetInstance().GetInputManager();
    auto* scene = golias::Engine::GetInstance().GetScene();
    
    // Game-specific input (only in Main scene)
    if (scene && scene->GetName() == "Main") {
        if (input.IsKeyJustPressed(SDLK_E)) {
            float x = (std::rand() % 10 - 5);
            float z = (std::rand() % 10 - 5);
            SpawnBox({x, 8.0f, z});
            spdlog::info("Spawned box at {}, {}, {}", x, 8.0f, z);
        }

        static float rotation = 0.0f;
        rotation += glm::radians(15.0f) * deltaTime;
        if (porsche) {
            porsche->SetRotation(glm::quat(glm::vec3(0.0f, rotation, 0.0f)));
        }

        if (input.IsKeyJustPressed(SDLK_T)) {
            auto& physicsDebugDrawer = golias::Engine::GetInstance().GetPhysicsManager();
            physicsDebugDrawer.SetDebugDrawEnabled(!physicsDebugDrawer.IsDebugDrawEnabled());
            spdlog::info("Toggled Physics Debug Draw: {}", physicsDebugDrawer.IsDebugDrawEnabled() ? "ON" : "OFF");
        }

        if (input.IsKeyJustPressed(SDLK_ESCAPE)) {
            spdlog::info("Returning to main menu...");
            golias::Scene::ChangeTo("scenes/menu.gscene");
        }
    }
}

void SandboxApplication::Destroy() {
    spdlog::info("GameApplication Destroy called.");
}
