#pragma once

#include "core/renderer/renderer.h"

/*!
 *  @brief  Assimp Object Loader
 *
 *  @note  Loads 3D models using the Assimp library and converts them into engine-compatible structures.
 *
 *  
 */
class ObjectLoader {
public:
    static MeshInstance3D load_mesh(const std::string& path);

    static Model load_model(const std::string& path, Renderer* renderer = nullptr);

private:
    static std::string get_directory(const std::string& path);


    static MeshInstance3D create_mesh(aiMesh* aiMesh);

    static Material load_material(const aiScene* scene, aiMesh* mesh,
                                  const std::string& directory, Renderer& renderer);

    static void load_colors(aiMaterial* aiMat, Material& mat);

    static void load_textures(aiMaterial* aiMat, const aiScene* scene,
                              const std::string& dir, Renderer& renderer, Material& mat);

    static void parse_bones(aiMesh* mesh, Model& model);


    static void parse_animations(const aiScene* scene, Model& model);
};


