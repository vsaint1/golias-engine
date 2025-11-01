#pragma once
#include "core/api/engine_api.h"


void create_mesh_entity(
    const char* name,
    const char* path,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale,
    const char* material_tag) {

    auto renderer = GEngine->get_renderer();

    if (!renderer->_meshes.contains(path)) {
        MeshInstance3D mesh = AssimpLoader::load_mesh(path);
        renderer->_meshes[path] = {mesh};
    }

    if (!renderer->_materials.contains(material_tag)) {
        spdlog::error("Material '{}' not registered!", material_tag);
        return;
    }

    GEngine->get_world().entity(name)
           .set(Transform3D{position, rotation, scale})
           .set(MeshRef{&renderer->_meshes[path][0]})
           .set(MaterialRef{&renderer->_materials[material_tag][0]});

    spdlog::info("MeshInstance3D entity '{}' created with material '{}'.", name, material_tag);
}


void create_model_entity(
    const char* name,
    const char* path,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale) {

    auto renderer = GEngine->get_renderer();


  if (!renderer->_meshes.contains(path) || !renderer->_materials.contains(path)) {
        Model model = AssimpLoader::load_model(path);
        renderer->_meshes[path]    = model.meshes;
        renderer->_materials[path] = model.materials;
    }

    const auto& meshes    = renderer->_meshes[path];
    const auto& materials = renderer->_materials[path];

    auto entity = GEngine->get_world().entity(name);

    for (size_t i = 0; i < meshes.size(); ++i) {
        entity.child()
            .set(Transform3D{position, rotation, scale})
            .set(MeshRef{&meshes[i]})
            .set(MaterialRef{&materials[i]});
    }

    spdlog::info("MeshInstance3D entity '{}' created with {} mesh parts.", name, meshes.size());
}


void create_material(const char* name, const Material& material) {
    GEngine->get_renderer()->_materials[name] = {material};
    spdlog::info("Material '{}' created and registered.", name);
}
