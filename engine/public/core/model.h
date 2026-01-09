#pragma once

#include <string>
#include <string_view>
#include <memory>

namespace golias {

    class GameObject;
    class Scene;

    /**
     * @brief Model loading and parsing utility
     * 
     * Supports loading GLTF, GLB, and OBJ model formats.
     * Creates GameObject hierarchies with appropriate components (meshes, materials, animations).
     */
    class Model {
    public:
        /**
         * @brief Load a 3D model from file
         * 
         * @param path Path to the model file (relative to assets directory)
         * @param scene Scene to create objects in
         * @return GameObject* Root object of the loaded model hierarchy, or nullptr on failure
         */
        static GameObject* Load(std::string_view path, Scene* scene);

    private:
        Model() = delete;
        
        // GLTF/GLB loading
        static GameObject* LoadGLTF(std::string_view path, Scene* scene);
        
        // OBJ loading
        static GameObject* LoadOBJ(std::string_view path, Scene* scene);

        // FBX loading (not implemented)
        static GameObject* LoadFBX(std::string_view path, Scene* scene) { return nullptr; }
    };

} // namespace golias