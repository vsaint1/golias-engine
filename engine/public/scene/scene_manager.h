#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <queue>

namespace golias {

    class Scene;

    /// @brief Transition types for scene changes
    enum class ESceneTransitionType {
        NONE,           /// Instant switch
        FADE,           /// Fade to black then fade in
        CROSSFADE,      /// Crossfade between scenes
        SLIDE_LEFT,     /// Slide left
        SLIDE_RIGHT,    /// Slide right
        SLIDE_UP,       /// Slide up
        SLIDE_DOWN      /// Slide down
    };

    /// @brief Load scene mode
    enum class ELoadSceneMode {
        SINGLE,         /// Unload all current scenes and load new one
        ADDITIVE        /// Load scene additively (keep current scenes)
    };

    using SceneLoadedCallback = std::function<void(Scene*)>;
    using SceneUnloadedCallback = std::function<void(const std::string&)>;
    using SceneTransitionCallback = std::function<void()>;

    /// @brief Manages scene loading, unloading, and transitions 
    class SceneManager {
    public:
        static SceneManager& GetInstance();

        /// @brief Load a scene by path
        /// @param scenePath Path to the scene file
        /// @param mode Load mode (single or additive)
        /// @param transition Transition type
        /// @param transitionDuration Duration of the transition in seconds
        void LoadScene(const std::string& scenePath, 
                      ELoadSceneMode mode = ELoadSceneMode::SINGLE,
                      ESceneTransitionType transition = ESceneTransitionType::NONE,
                      float transitionDuration = 0.5f);

        /// @brief Load a scene by index (from registered scenes)
        /// @param sceneIndex Index of the scene in the build settings
        /// @param mode Load mode
        /// @param transition Transition type
        /// @param transitionDuration Duration of the transition
        void LoadScene(int sceneIndex,
                      ELoadSceneMode mode = ELoadSceneMode::SINGLE,
                      ESceneTransitionType transition = ESceneTransitionType::NONE,
                      float transitionDuration = 0.5f);

        /// @brief Load scene asynchronously
        /// @param scenePath Path to the scene
        /// @param callback Called when scene is loaded
        void LoadSceneAsync(const std::string& scenePath, SceneLoadedCallback callback = nullptr);

        /// @brief Unload a scene by name
        /// @param sceneName Name of the scene to unload
        void UnloadScene(const std::string& sceneName);

        /// @brief Get the currently active scene
        Scene* GetActiveScene() const;

        /// @brief Set the active scene (for multi-scene setups)
        void SetActiveScene(Scene* pScene);

        /// @brief Get scene by name
        Scene* GetSceneByName(const std::string& name) const;

        /// @brief Get the count of loaded scenes
        int GetSceneCount() const;

        /// @brief Get scene at index
        Scene* GetSceneAt(int index) const;

        /// @brief Register a scene for indexed loading
        /// @param index Build index for the scene
        /// @param scenePath Path to the scene file
        void RegisterScene(int index, const std::string& scenePath);

        /// @brief Get registered scene path by index
        const std::string& GetScenePathByIndex(int index) const;

        /// @brief Get the total number of registered scenes
        int GetRegisteredSceneCount() const;

        /// @brief Check if a transition is in progress
        bool IsTransitioning() const;

        /// @brief Get the current transition progress (0.0 to 1.0)
        float GetTransitionProgress() const;

        /// @brief Update the scene manager (called from engine loop)
        void Update(float deltaTime);

        /// @brief Draw transition effects (called after scene rendering)
        void DrawTransition();

        // Event callbacks
        SceneLoadedCallback OnSceneLoaded;
        SceneUnloadedCallback OnSceneUnloaded;
        SceneTransitionCallback OnTransitionStarted;
        SceneTransitionCallback OnTransitionComplete;

    private:
        SceneManager() = default;
        ~SceneManager() = default;
        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;

        void ProcessSceneLoad();
        void StartTransition();
        void UpdateTransition(float deltaTime);
        void CompleteTransition();

        struct PendingSceneLoad {
            std::string scenePath;
            ELoadSceneMode mode;
            ESceneTransitionType transition;
            float transitionDuration;
            SceneLoadedCallback callback;
        };

        std::vector<std::shared_ptr<Scene>> loadedScenes;
        Scene* activeScene = nullptr;
        
        std::unordered_map<int, std::string> registeredScenes;
        std::queue<PendingSceneLoad> pendingLoads;

        // Transition state
        bool isTransitioning = false;
        ESceneTransitionType currentTransition = ESceneTransitionType::NONE;
        float transitionDuration = 0.5f;
        float transitionTimer = 0.0f;
        float transitionProgress = 0.0f;
        bool transitionHalfway = false;
        
        std::shared_ptr<Scene> transitionFromScene = nullptr;
        std::shared_ptr<Scene> transitionToScene = nullptr;

        static const std::string emptyString;
    };

} // namespace golias
