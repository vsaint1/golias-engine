#pragma once
#include "core/api/engine_api.h"

void generate_unique_name(flecs::entity& e, const char* default_name, const char* custom_name) {
    if (custom_name) {
        e.set_name(custom_name);
    } else {
        std::string unique_name = std::string(default_name) + "_" + std::to_string(e.id());
        e.set_name(unique_name.c_str());
    }
}

GameObject create_camera_3d_entity(const char* name, const glm::vec3& position, const glm::vec3& rotation, float fov, float near_plane,
                                   float far_plane) {


    const auto& world = GEngine->get_world();

    flecs::entity e = world.entity().add<Camera3D>().set<Transform3D>({position, rotation});

    generate_unique_name(e, "Camera3D", name);

    const GameObject go(e);

    return go;
}

GameObject create_directional_light_3d_entity(const char* name, const glm::vec3& direction, const glm::vec3& color, float intensity,
                                              bool cast_shadows, float shadow_distance, float shadow_near, float shadow_far) {
    const auto& world = GEngine->get_world();

    flecs::entity e = world.entity().set<Transform3D>({}).set<DirectionalLight3D>(
        {glm::normalize(direction), glm::vec3(color), intensity, cast_shadows, shadow_distance, shadow_near, shadow_far});

    generate_unique_name(e, "DirectionalLight3D", name);

    const GameObject go(e);

    return go;
}

GameObject create_spot_light_3d_entity(const char* name, const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color,
                                       float inner_cutoff, float outer_cutoff, float intensity) {
    const auto& world = GEngine->get_world();

    flecs::entity entity = world.entity()
                               .set<Transform3D>({position})
                               .set<SpotLight3D>({glm::vec3(direction), glm::vec3(color), intensity, inner_cutoff, outer_cutoff});

    generate_unique_name(entity, "SpotLight3D", name);

    const GameObject go(entity);
    return go;
}

GameObject create_mesh_3d_entity(const char* name, const char* path, const glm::vec3& position, const glm::vec3& rotation,
                                 const glm::vec3& scale, const char* material_tag) {

    const auto renderer = GEngine->get_renderer();

    if (!renderer->_meshes.contains(path)) {
        MeshInstance3D mesh     = AssimpLoader::load_mesh(path);
        renderer->_meshes[path] = {mesh};
    }

    if (!renderer->_materials.contains(material_tag)) {
        spdlog::error("Material '{}' not registered!", material_tag);
        return GameObject{};
    }

    auto entity = GEngine->get_world()
                      .entity()
                      .set(Transform3D{position, rotation, scale})
                      .set(MeshRef{&renderer->_meshes[path][0]})
                      .set(MaterialRef{&renderer->_materials[material_tag][0]});

    generate_unique_name(entity, "MeshInstance3D", name);

    spdlog::info("MeshInstance3D entity '{}' created with material '{}'.", name, material_tag);

    const GameObject go(entity);

    return go;
}


GameObject create_model_3d_entity(const char* name, const char* path, BlendMode blend_mode, const glm::vec3& position,
                                  const glm::vec3& rotation, const glm::vec3& scale) {

    auto renderer = GEngine->get_renderer();


    if (!renderer->_meshes.contains(path) || !renderer->_materials.contains(path)) {
        Model model = AssimpLoader::load_model(path);

        if (model.meshes.empty() || model.materials.empty()) {
            spdlog::error("Failed to load model from path: {}", path);
            return GameObject{};
        }

        renderer->_meshes[path]    = model.meshes;
        renderer->_materials[path] = model.materials;
    }

    const auto& meshes = renderer->_meshes[path];
    auto& materials    = renderer->_materials[path];

    for (auto& material : materials) {
        material.blend_mode = blend_mode;
        spdlog::debug("Material blend mode set to: {}", static_cast<int>(blend_mode));
    }

    auto entity = GEngine->get_world().entity(name);

    for (size_t i = 0; i < meshes.size(); ++i) {
        entity.child().set(Transform3D{position, rotation, scale}).set(MeshRef{&meshes[i]}).set(MaterialRef{&materials[i]});
    }

    spdlog::info("MeshInstance3D entity '{}' created with {} mesh parts.", name, meshes.size());
    const GameObject go(entity);

    return go;
}


GameObject create_camera_2d_entity(const char* name, const glm::vec2& position, float rotation, float zoom) {

    const auto& world = GEngine->get_world();

    Camera2D camera;
    camera.zoom = zoom;

    flecs::entity e = world.entity().set(camera).set(Transform2D{position, glm::vec2(1.0f), rotation});

    generate_unique_name(e, "Camera2D", name);

    const GameObject go(e);

    return go;
}


void create_material(const char* name, Material material) {
    material.update_feature_flags();
    GEngine->get_renderer()->_materials[name] = {material};
    spdlog::info("Material '{}' created and registered.", name);
}
