#pragma once

#include <string>
#include <string_view>
#include <memory>

namespace golias {

    class GameObject;
    class Scene;

    enum class ModelFormat { UNKNOWN, GLTF, GLB, OBJ };

    /**
     * @brief Model loading and parsing utility
     * 
     * Supports loading 3D models from various formats (GLTF, OBJ, FBX, etc.) using Assimp.
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
        
        static GameObject* LoadAssimp(std::string_view path, Scene* scene);
        
    };

} // namespace golias