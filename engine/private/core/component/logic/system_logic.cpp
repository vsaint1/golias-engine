#include "core/component/logic/system_logic.h"

#include "core/binding/lua.h"
#include "core/engine.h"


int sort_by_z_index(flecs::entity_t e1, const Transform2D* t1, flecs::entity_t e2, const Transform2D* t2) {
    (void) e1;
    (void) e2;
    return t1->z_index - t2->z_index;
}

void render_world_2d_system(flecs::entity e, Camera2D& camera) {
    if (!e.is_valid()) {
        return;
    }

    auto& world = GEngine->get_world();

    auto q = world.query_builder<Transform2D>().order_by(sort_by_z_index).build();

    q.each([&](flecs::entity e, Transform2D& t) {
        update_transforms_system(e, t);


        if (e.has<Shape2D>()) {
            render_primitives_system(t, e.get_mut<Shape2D>());
        }

        if (e.has<Sprite2D>()) {
            render_sprites_system(t, e.get_mut<Sprite2D>());
        }

        if (e.has<Label2D>()) {
            render_labels_system(t, e.get_mut<Label2D>());
        }
    });
}

void render_primitives_system(Transform2D& t, Shape2D& s) {
    switch (s.type) {
    case ShapeType::TRIANGLE:
        GEngine->get_renderer()->draw_triangle(t, s.size.x, s.color, s.filled);
        break;
    case ShapeType::RECTANGLE:
        GEngine->get_renderer()->draw_rect(t, s.size.x, s.size.y, s.color, s.filled);
        break;
    case ShapeType::CIRCLE:
        GEngine->get_renderer()->draw_circle(t, s.radius, s.color, s.filled);
        break;
    case ShapeType::LINE:
        GEngine->get_renderer()->draw_line(t, s.end, s.color);
        break;
    case ShapeType::POLYGON:
        GEngine->get_renderer()->draw_polygon(t, s.vertices, s.color, s.filled);
        break;
    }
}

void render_labels_system(Transform2D& t, Label2D& l) {

    // LOG_INFO("Rendering label: %s at position (%.2f, %.2f)", l.text.c_str(), t.world_position.x, t.world_position.y);
    GEngine->get_renderer()->draw_text(t, l.color, l.font_name.c_str(), "%s", l.text.c_str());
}

void render_sprites_system(Transform2D& t, Sprite2D& sprite) {

    if (!sprite.texture_name.empty()) {

        auto texture = GEngine->get_renderer()->load_texture(sprite.texture_name);
        if (texture) {
            glm::vec4 source = sprite.source;

            glm::vec4 dest = {0, 0, source.z, source.w};

            GEngine->get_renderer()->draw_texture(t, texture.get(), dest, source, sprite.flip_h, sprite.flip_v, sprite.color);
        }
    }
}


void update_transforms_system(flecs::entity e, Transform2D& t) {
    auto parent = e.parent();
    if (parent.is_valid() && parent.has<Transform2D>()) {
        const Transform2D& parent_t = parent.get<Transform2D>();

        // Update world position, scale and rotation based on parent's transform
        t.world_position = parent_t.world_position + t.position;

        t.world_scale = parent_t.world_scale * t.scale;

        t.world_rotation = parent_t.world_rotation + t.rotation;
    } else {
        // No parent with Transform2D, so local is world
        t.world_position = t.position;
        t.world_scale    = t.scale;
        t.world_rotation = t.rotation;
    }

    // LOG_INFO("Entity: %s, Local Pos: (%.2f, %.2f), World Pos: (%.2f, %.2f)", e.name().c_str(), t.position.x, t.position.y, t.world_position.x,
    //          t.world_position.y);
}


const aiNodeAnim* find_node_anim(const aiAnimation* animation, const std::string& nodeName) {
    for (unsigned int i = 0; i < animation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (std::string(nodeAnim->mNodeName.C_Str()) == nodeName) {
            return nodeAnim;
        }
    }
    return nullptr;
}

glm::vec3 interpolate_pos(float animTime, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumPositionKeys == 1) {
        aiVector3D v = nodeAnim->mPositionKeys[0].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }

    unsigned int positionIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
        if (animTime < (float) nodeAnim->mPositionKeys[i + 1].mTime) {
            positionIndex = i;
            break;
        }
    }

    unsigned int nextIndex = positionIndex + 1;
    float deltaTime        = (float) (nodeAnim->mPositionKeys[nextIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
    float factor           = (animTime - (float) nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;

    const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
    const aiVector3D& end   = nodeAnim->mPositionKeys[nextIndex].mValue;

    glm::vec3 startVec(start.x, start.y, start.z);
    glm::vec3 endVec(end.x, end.y, end.z);

    return glm::mix(startVec, endVec, factor);
}

glm::quat interpolate_rot(float animTime, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumRotationKeys == 1) {
        aiQuaternion q = nodeAnim->mRotationKeys[0].mValue;
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    unsigned int rotationIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
        if (animTime < (float) nodeAnim->mRotationKeys[i + 1].mTime) {
            rotationIndex = i;
            break;
        }
    }

    unsigned int nextIndex = rotationIndex + 1;
    float deltaTime        = (float) (nodeAnim->mRotationKeys[nextIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime);
    float factor           = (animTime - (float) nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;

    const aiQuaternion& startQuat = nodeAnim->mRotationKeys[rotationIndex].mValue;
    const aiQuaternion& endQuat   = nodeAnim->mRotationKeys[nextIndex].mValue;

    glm::quat start(startQuat.w, startQuat.x, startQuat.y, startQuat.z);
    glm::quat end(endQuat.w, endQuat.x, endQuat.y, endQuat.z);

    return glm::slerp(start, end, factor);
}

glm::vec3 interpolate_scale(float animTime, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumScalingKeys == 1) {
        aiVector3D v = nodeAnim->mScalingKeys[0].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }

    unsigned int scaleIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
        if (animTime < (float) nodeAnim->mScalingKeys[i + 1].mTime) {
            scaleIndex = i;
            break;
        }
    }

    unsigned int nextIndex = scaleIndex + 1;
    float deltaTime        = (float) (nodeAnim->mScalingKeys[nextIndex].mTime - nodeAnim->mScalingKeys[scaleIndex].mTime);
    float factor           = (animTime - (float) nodeAnim->mScalingKeys[scaleIndex].mTime) / deltaTime;

    const aiVector3D& start = nodeAnim->mScalingKeys[scaleIndex].mValue;
    const aiVector3D& end   = nodeAnim->mScalingKeys[nextIndex].mValue;

    glm::vec3 startVec(start.x, start.y, start.z);
    glm::vec3 endVec(end.x, end.y, end.z);

    return glm::mix(startVec, endVec, factor);
}

void read_node_hierarchy(float animTime, const aiNode* node, const glm::mat4& parentTransform, const aiAnimation* animation,
                         const Model& model, std::unordered_map<std::string, glm::mat4>& boneTransforms) {
    std::string nodeName(node->mName.C_Str());
    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    const aiNodeAnim* nodeAnim = find_node_anim(animation, nodeName);

    if (nodeAnim) {
        glm::vec3 position = interpolate_pos(animTime, nodeAnim);
        glm::quat rotation = interpolate_rot(animTime, nodeAnim);
        glm::vec3 scale    = interpolate_scale(animTime, nodeAnim);

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationMatrix    = glm::mat4_cast(rotation);
        glm::mat4 scaleMatrix       = glm::scale(glm::mat4(1.0f), scale);

        nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;
    boneTransforms[nodeName]  = globalTransform;

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        read_node_hierarchy(animTime, node->mChildren[i], globalTransform, animation, model, boneTransforms);
    }
}

void update_animation(Model& model, Animation3D& anim, float deltaTime) {
    if (!model.scene || !model.scene->HasAnimations() || !anim.playing) {
        return;
    }

    if (anim.current_animation >= (int) model.scene->mNumAnimations) {
        anim.current_animation = 0;
    }

    const aiAnimation* animation = model.scene->mAnimations[anim.current_animation];

    anim.time += deltaTime * anim.speed;

    float duration       = (float) animation->mDuration;
    float ticksPerSecond = (float) (animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f);
    float timeInTicks    = anim.time * ticksPerSecond;

    if (anim.loop) {
        timeInTicks = fmod(timeInTicks, duration);
    } else {
        if (timeInTicks >= duration) {
            timeInTicks  = duration;
            anim.playing = false;
        }
    }

    std::unordered_map<std::string, glm::mat4> nodeTransforms;
    read_node_hierarchy(timeInTicks, model.scene->mRootNode, glm::mat4(1.0f), animation, model, nodeTransforms);

    constexpr int MAX_BONES = 200;
    anim.bone_transforms.resize(MAX_BONES, glm::mat4(1.0f));

    for (const auto& mesh : model.meshes) {
        if (!mesh->has_bones) {
            continue;
        }

        for (size_t i = 0; i < mesh->bones.size() && i < MAX_BONES; i++) {
            const Bone& bone = mesh->bones[i];

            auto it = nodeTransforms.find(bone.name);
            if (it != nodeTransforms.end()) {
                anim.bone_transforms[i] = model.global_inverse_transform * it->second * bone.offset_matrix;
            } else {
                anim.bone_transforms[i] = glm::mat4(1.0f);
            }
        }
    }
}

void render_world_3d_system(flecs::entity e, Camera3D& camera) {


    if (!e.is_valid()) {
        return;
    }


    const auto& window = GEngine->get_config().get_window();

    // Render all 3D models in the scene (non-animated)
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, const Model& model) {
        // Skip entities with Animation3D component (they're handled by animation system)
        if (e.has<Animation3D>()) {
            return;
        }

        // TODO: create system to handle model loading
        auto m = GEngine->get_renderer()->load_model(model.path.c_str());
        GEngine->get_renderer()->draw_model(t, m.get());
    });

    // Render all cubes in the scene
    GEngine->get_world().each(
        [&](flecs::entity e, Transform3D& t, const MeshInstance3D& cube) { GEngine->get_renderer()->draw_mesh(t, cube); });

    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, const Camera3D& cam) {
        GEngine->get_renderer()->flush(cam.get_view(t), cam.get_projection(window.width, window.height));
    });
}

void animation_system(flecs::entity e, Model& model, Animation3D& anim, Transform3D& transform) {
    if (!model.scene && !model.path.empty()) {
        auto loaded = GEngine->get_renderer()->load_model(model.path.c_str());
        if (loaded) {
            model.importer                 = loaded->importer;
            model.scene                    = loaded->scene;
            model.global_inverse_transform = loaded->global_inverse_transform;
            model.meshes                   = loaded->meshes;

            // LOG_DEBUG("Loaded model for animation: %s", model.path.c_str());
            // if (model.scene && model.scene->HasAnimations()) {
            //     LOG_DEBUG("  Found %u animations:", model.scene->mNumAnimations);
            //     for (unsigned int i = 0; i < model.scene->mNumAnimations; i++) {
            //         LOG_DEBUG("    [%u] %s", i, model.scene->mAnimations[i]->mName.C_Str());
            //     }
            // }
        }
    }

    update_animation(model, anim, GEngine->get_timer().delta);

    if (!anim.bone_transforms.empty()) {
        GEngine->get_renderer()->draw_animated_model(transform, &model, anim.bone_transforms.data(), anim.bone_transforms.size());
    }
}

void setup_scripts_system(flecs::entity e, Script& script) {
    // Create a NEW lua_State for THIS script
    script.lua_state = luaL_newstate();
    luaL_openlibs(script.lua_state);

    // Setup package path for this state
    lua_getglobal(script.lua_state, "package");
    lua_getfield(script.lua_state, -1, "path");
    std::string path = lua_tostring(script.lua_state, -1);
    lua_pop(script.lua_state, 1);
    path.append(";./res/scripts/?.lua;res/scripts/?.lua;./?.lua");
    lua_pushstring(script.lua_state, path.c_str());
    lua_setfield(script.lua_state, -2, "path");
    lua_pop(script.lua_state, 1);

    generate_bindings(script.lua_state);

    script.ready_called = false;

    // Load the Lua script from file
    FileAccess lua_file(script.path, ModeFlags::READ);
    const std::string& lua_script = lua_file.get_file_as_str();

    // Load & execute script file
    if (luaL_loadstring(script.lua_state, lua_script.c_str()) || lua_pcall(script.lua_state, 0, 0, 0)) {
        const char* err = lua_tostring(script.lua_state, -1);
        LOG_ERROR("Failed to load script %s: %s", script.path.c_str(), err);
        lua_pop(script.lua_state, 1);
        return;
    }

    generate_bindings(script.lua_state);

    push_entity_to_lua(script.lua_state, e);

    // Call _ready() if it exists
    lua_getglobal(script.lua_state, "_ready");
    if (lua_isfunction(script.lua_state, -1)) {
        if (lua_pcall(script.lua_state, 0, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(script.lua_state, -1);
            LOG_ERROR("Error in _ready() of %s: %s", script.path.c_str(), err);
            lua_pop(script.lua_state, 1);
        } else {
            script.ready_called = true;
        }
    } else {
        lua_pop(script.lua_state, 1);
    }
}

void process_event_scripts_system(const Script& script, const SDL_Event& event) {
    if (!script.ready_called || !script.lua_state) {
        return;
    }

    lua_getglobal(script.lua_state, "_input");

    if (!lua_isfunction(script.lua_state, -1)) {
        lua_pop(script.lua_state, 1);
        return;
    }

    push_sdl_event_to_lua(script.lua_state, event);

    if (lua_pcall(script.lua_state, 1, 0, 0) != LUA_OK) {
        const char* err_msg = lua_tostring(script.lua_state, -1);
        LOG_ERROR("Error in _input() of %s: %s", script.path.c_str(), err_msg);
        lua_pop(script.lua_state, 1);
    }
}


void process_scripts_system(Script& script) {
    if (!script.ready_called || !script.lua_state) {
        return;
    }

    // Call _process
    lua_getglobal(script.lua_state, "_process");
    if (lua_isfunction(script.lua_state, -1)) {
        lua_pushnumber(script.lua_state, static_cast<lua_Number>(GEngine->get_timer().delta));

        if (lua_pcall(script.lua_state, 1, 0, 0) != LUA_OK) {
            const char* err_msg = lua_tostring(script.lua_state, -1);
            LOG_ERROR("Error in _process() of %s: %s", script.path.c_str(), err_msg);
            lua_pop(script.lua_state, 1);
        }
    } else {
        lua_pop(script.lua_state, 1);
    }

    // Call _draw
    lua_getglobal(script.lua_state, "_draw");
    if (lua_isfunction(script.lua_state, -1)) {
        if (lua_pcall(script.lua_state, 0, 0, 0) != LUA_OK) {
            const char* err_msg = lua_tostring(script.lua_state, -1);
            LOG_ERROR("Error in _draw() of %s: %s", script.path.c_str(), err_msg);
            lua_pop(script.lua_state, 1);
        }
    } else {
        lua_pop(script.lua_state, 1);
    }
}

void scene_manager_system(flecs::world& world) {
    world.observer<SceneChangeRequest>("SceneChangeRequest_Observer")
        .event(flecs::OnSet)
        .each([&](flecs::iter& it, size_t i, SceneChangeRequest& req) {
            LOG_INFO("Scene requested: %s", req.name.c_str());

            auto new_scene = world.lookup(req.name.c_str());

            if (new_scene.is_valid() && new_scene.has<tags::Scene>()) {

                world.each([&](flecs::entity e, tags::Scene) {
                    e.add(flecs::Disabled);
                    e.remove<tags::ActiveScene>();
                });

                new_scene.remove(flecs::Disabled);
                new_scene.add<tags::ActiveScene>();
                LOG_INFO("Switched to scene: %s", req.name.c_str());
            } else {
                LOG_WARN("Scene '%s' not found", req.name.c_str());
            }

            it.entity(i).remove<SceneChangeRequest>();
        });
}
