#pragma once
#include "core/api/engine_api.h"


void create_mesh_entity(const char* name, const char* path, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale,
                        const char* material_tag) {

    const auto renderer = GEngine->get_renderer();

    if (!renderer->_meshes.contains(path)) {
        MeshInstance3D mesh     = AssimpLoader::load_mesh(path);
        renderer->_meshes[path] = {mesh};
    }

    if (!renderer->_materials.contains(material_tag)) {
        spdlog::error("Material '{}' not registered!", material_tag);
        return;
    }

    GEngine->get_world()
        .entity(name)
        .set(Transform3D{position, rotation, scale})
        .set(MeshRef{&renderer->_meshes[path][0]})
        .set(MaterialRef{&renderer->_materials[material_tag][0]});

    spdlog::info("MeshInstance3D entity '{}' created with material '{}'.", name, material_tag);
}


void create_model_entity(const char* name, const char* path,  BlendMode blend_mode,const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {

    auto renderer = GEngine->get_renderer();


    if (!renderer->_meshes.contains(path) || !renderer->_materials.contains(path)) {
        Model model                = AssimpLoader::load_model(path);
        renderer->_meshes[path]    = model.meshes;
        renderer->_materials[path] = model.materials;
    }

    const auto& meshes    = renderer->_meshes[path];
    auto& materials = renderer->_materials[path];

    for (auto& material : materials) {
        material.blend_mode = blend_mode;
        spdlog::debug("Material blend mode set to: {}", static_cast<int>(blend_mode));
    }

    auto entity = GEngine->get_world().entity(name);

    for (size_t i = 0; i < meshes.size(); ++i) {
        entity.child().set(Transform3D{position, rotation, scale}).set(MeshRef{&meshes[i]}).set(MaterialRef{&materials[i]});
    }

    spdlog::info("MeshInstance3D entity '{}' created with {} mesh parts.", name, meshes.size());
}


void create_model_entity_with_blend(const char* name, const char* path, BlendMode blend_mode,
                                   const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {

    auto renderer = GEngine->get_renderer();

    if (!renderer->_meshes.contains(path) || !renderer->_materials.contains(path)) {
        Model model                = AssimpLoader::load_model(path);
        renderer->_meshes[path]    = model.meshes;
        renderer->_materials[path] = model.materials;
    }

    const auto& meshes    = renderer->_meshes[path];
    auto& materials = renderer->_materials[path];

    // Apply blend mode to all materials
    for (auto& material : materials) {
        material.blend_mode = blend_mode;
        spdlog::debug("Material blend mode set to: {}", static_cast<int>(blend_mode));
    }

    auto entity = GEngine->get_world().entity(name);

    for (size_t i = 0; i < meshes.size(); ++i) {
        entity.child().set(Transform3D{position, rotation, scale}).set(MeshRef{&meshes[i]}).set(MaterialRef{&materials[i]});
    }

    spdlog::info("MeshInstance3D entity '{}' created with {} mesh parts (blend_mode: {}).", 
                 name, meshes.size(), static_cast<int>(blend_mode));
}


void create_material(const char* name,  Material material) {
    material.update_feature_flags();
    GEngine->get_renderer()->_materials[name] = {material};
    spdlog::info("Material '{}' created and registered.", name);

}
