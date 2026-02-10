#include "scene/scene_manager.h"

#include "core/engine.h"
#include "core/input/canvas_input_manager.h"
#include "physics/3d/physics_manager.h"
#include "scene/scene.h"
#include "core/graphics/scene_renderer.h"
#include "core/graphics/rendering_device.h"

namespace golias {

    const std::string SceneManager::emptyString = "";

    SceneManager& SceneManager::GetInstance() {
        static SceneManager instance;
        return instance;
    }

    void SceneManager::LoadScene(const std::string& scenePath, 
                                  ELoadSceneMode mode,
                                  ESceneTransitionType transition,
                                  float duration) {
        PendingSceneLoad load;
        load.scenePath = scenePath;
        load.mode = mode;
        load.transition = transition;
        load.transitionDuration = duration;
        load.callback = nullptr;

        pendingLoads.push(load);

        if (!isTransitioning && pendingLoads.size() == 1) {
            ProcessSceneLoad();
        }
    }

    void SceneManager::LoadScene(int sceneIndex,
                                  ELoadSceneMode mode,
                                  ESceneTransitionType transition,
                                  float duration) {
        auto it = registeredScenes.find(sceneIndex);
        if (it != registeredScenes.end()) {
            LoadScene(it->second, mode, transition, duration);
        } else {
            spdlog::error("SceneManager::LoadScene Scene at index {} is not registered", sceneIndex);
        }
    }

    void SceneManager::LoadSceneAsync(const std::string& scenePath, SceneLoadedCallback callback) {
        PendingSceneLoad load;
        load.scenePath = scenePath;
        load.mode = ELoadSceneMode::SINGLE;
        load.transition = ESceneTransitionType::FADE;
        load.transitionDuration = 0.5f;
        load.callback = callback;

        pendingLoads.push(load);

        if (!isTransitioning && pendingLoads.size() == 1) {
            ProcessSceneLoad();
        }
    }

    void SceneManager::UnloadScene(const std::string& sceneName) {
        auto it = std::find_if(loadedScenes.begin(), loadedScenes.end(),
            [&sceneName](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == sceneName;
            });

        if (it != loadedScenes.end()) {
            if (activeScene == it->get()) {
                activeScene = loadedScenes.size() > 1 ? loadedScenes[0].get() : nullptr;
            }

            std::string name = (*it)->GetName();
            loadedScenes.erase(it);

            if (OnSceneUnloaded) {
                OnSceneUnloaded(name);
            }

            spdlog::info("SceneManager::UnloadScene Unloaded scene '{}'", name);
        }
    }

    Scene* SceneManager::GetActiveScene() const {
        return activeScene;
    }

    void SceneManager::SetActiveScene(Scene* pScene) {
        activeScene = pScene;
    }

    Scene* SceneManager::GetSceneByName(const std::string& name) const {
        auto it = std::find_if(loadedScenes.begin(), loadedScenes.end(),
            [&name](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == name;
            });

        return (it != loadedScenes.end()) ? it->get() : nullptr;
    }

    int SceneManager::GetSceneCount() const {
        return static_cast<int>(loadedScenes.size());
    }

    Scene* SceneManager::GetSceneAt(int index) const {
        if (index >= 0 && index < static_cast<int>(loadedScenes.size())) {
            return loadedScenes[index].get();
        }
        return nullptr;
    }

    void SceneManager::RegisterScene(int index, const std::string& scenePath) {
        registeredScenes[index] = scenePath;
        spdlog::info("SceneManager::RegisterScene Registered scene '{}' at index {}", scenePath, index);
    }

    const std::string& SceneManager::GetScenePathByIndex(int index) const {
        auto it = registeredScenes.find(index);
        return (it != registeredScenes.end()) ? it->second : emptyString;
    }

    int SceneManager::GetRegisteredSceneCount() const {
        return static_cast<int>(registeredScenes.size());
    }

    bool SceneManager::IsTransitioning() const {
        return isTransitioning;
    }

    float SceneManager::GetTransitionProgress() const {
        return transitionProgress;
    }

    void SceneManager::Update(float deltaTime) {
        if (isTransitioning) {
            UpdateTransition(deltaTime);
        }
    }

    void SceneManager::DrawTransition() {
        if (!isTransitioning || currentTransition == ESceneTransitionType::NONE) {
            return;
        }

        auto& renderer = Engine::GetInstance().GetSceneRenderer();
        const auto& viewport = renderer.GetRenderingDevice()->GetViewport();

        float alpha = 0.0f;
        
        switch (currentTransition) {
            case ESceneTransitionType::FADE:
                // Fade to black at 0.5, then fade in
                if (transitionProgress < 0.5f) {
                    alpha = transitionProgress * 2.0f;
                } else {
                    alpha = 1.0f - (transitionProgress - 0.5f) * 2.0f;
                }
                break;

            case ESceneTransitionType::CROSSFADE:
                // Alpha based on direct progress
                alpha = 0.0f; // Crossfade handled differently
                break;

            default:
                break;
        }

        if (alpha > 0.01f) {
            // Draw a full-screen black overlay
            // This would require submitting a screen overlay command to the renderer
            // For now, we'll use the canvas system
            glm::vec4 overlayColor(0.0f, 0.0f, 0.0f, alpha);
            
            // The actual drawing would be done through a dedicated overlay system
            // This is a placeholder for the concept
        }
    }

    void SceneManager::ProcessSceneLoad() {
        if (pendingLoads.empty()) {
            return;
        }

        const PendingSceneLoad& load = pendingLoads.front();

        if (load.transition != ESceneTransitionType::NONE) {
            currentTransition = load.transition;
            transitionDuration = load.transitionDuration;
            StartTransition();
        } else {
            // Instant load - clear canvas input manager first
            auto& canvasInputManager = Engine::GetInstance().GetCanvasInputManager();
            canvasInputManager.SetActiveCanvas(nullptr);
            
            std::shared_ptr<Scene> newScene = Scene::Load(load.scenePath);
            
            if (newScene) {
                if (load.mode == ELoadSceneMode::SINGLE) {
                    Engine::GetInstance().GetPhysicsManager().ClearCollisionState();
                    loadedScenes.clear();
                }
                
                loadedScenes.push_back(newScene);
                activeScene = newScene.get();
                
                Engine::GetInstance().SetScene(newScene);

                if (load.callback) {
                    load.callback(newScene.get());
                }

                if (OnSceneLoaded) {
                    OnSceneLoaded(newScene.get());
                }

                spdlog::info("SceneManager::ProcessSceneLoad Loaded scene '{}'", newScene->GetName());
            } else {
                spdlog::error("SceneManager::ProcessSceneLoad Failed to load scene '{}'", load.scenePath);
            }

            pendingLoads.pop();

            if (!pendingLoads.empty()) {
                ProcessSceneLoad();
            }
        }
    }

    void SceneManager::StartTransition() {
        isTransitioning = true;
        transitionTimer = 0.0f;
        transitionProgress = 0.0f;
        transitionHalfway = false;
        
        // Store current scene for transition
        if (!loadedScenes.empty()) {
            transitionFromScene = loadedScenes.back();
        }

        if (OnTransitionStarted) {
            OnTransitionStarted();
        }

        spdlog::info("SceneManager::StartTransition Started scene transition");
    }

    void SceneManager::UpdateTransition(float deltaTime) {
        transitionTimer += deltaTime;
        transitionProgress = glm::clamp(transitionTimer / transitionDuration, 0.0f, 1.0f);

        // At halfway point, load the new scene
        if (!transitionHalfway && transitionProgress >= 0.5f) {
            transitionHalfway = true;

            if (!pendingLoads.empty()) {
                const PendingSceneLoad& load = pendingLoads.front();
                
                // Clear canvas input manager before loading new scene
                auto& canvasInputManager = Engine::GetInstance().GetCanvasInputManager();
                canvasInputManager.SetActiveCanvas(nullptr);
                
                std::shared_ptr<Scene> newScene = Scene::Load(load.scenePath);
                
                if (newScene) {
                    if (load.mode == ELoadSceneMode::SINGLE) {
                        // Clear physics collision tracking before destroying old scene
                        // objects to prevent dangling GameObject* pointers
                        Engine::GetInstance().GetPhysicsManager().ClearCollisionState();
                        loadedScenes.clear();
                    }
                    
                    loadedScenes.push_back(newScene);
                    activeScene = newScene.get();
                    transitionToScene = newScene;
                    
                    Engine::GetInstance().SetScene(newScene);

                    if (load.callback) {
                        load.callback(newScene.get());
                    }

                    if (OnSceneLoaded) {
                        OnSceneLoaded(newScene.get());
                    }

                    spdlog::info("SceneManager::UpdateTransition Loaded scene '{}' during transition", newScene->GetName());
                }
            }
        }

        // Complete transition
        if (transitionProgress >= 1.0f) {
            CompleteTransition();
        }
    }

    void SceneManager::CompleteTransition() {
        isTransitioning = false;
        transitionFromScene = nullptr;
        transitionToScene = nullptr;
        currentTransition = ESceneTransitionType::NONE;

        if (!pendingLoads.empty()) {
            pendingLoads.pop();
        }

        if (OnTransitionComplete) {
            OnTransitionComplete();
        }

        spdlog::info("SceneManager::CompleteTransition Scene transition completed");

        // Process next pending load if any
        if (!pendingLoads.empty()) {
            ProcessSceneLoad();
        }
    }

} // namespace golias
