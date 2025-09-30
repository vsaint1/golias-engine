#include "core/component/logic/system_logic.h"

#include "core/binding/lua.h"
#include "core/engine.h"


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

void render_world_3d_system(flecs::entity e, Camera3D& camera) {


    if (!e.is_valid() || !e.has<tags::MainCamera>()) {
        return;
    }

    const auto& window = GEngine->get_config().get_window();
    GEngine->get_renderer()->draw_environment(camera.get_view(), camera.get_projection(window.width, window.height));

    // Render all 3D models in the scene
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, const std::shared_ptr<Model>& model) {
        GEngine->get_renderer()->draw_model(t, model.get(), camera.get_view(), camera.get_projection(window.width, window.height),
                                            camera.position);
    });
}


void camera_touch_system(flecs::entity e, Camera3D& camera, const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        camera.zoom(static_cast<float>(event.wheel.y));
    }


    float xoffset = 0.0f;
    float yoffset = 0.0f;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {


        if (event.motion.state & SDL_BUTTON_LMASK) {
            xoffset = static_cast<float>(event.motion.xrel);
            yoffset = static_cast<float>(event.motion.yrel);
        }
    }

    if (event.type == SDL_EVENT_FINGER_MOTION) {
        xoffset = static_cast<float>(event.tfinger.dx * 10);
        yoffset = static_cast<float>(event.tfinger.dy * 10);
    }

    camera.look_at(xoffset, -yoffset, 1.f);
}



void camera_keyboard_system(flecs::entity e, Camera3D& camera, const float delta) {


    const bool* state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_W]) {
        camera.move_forward(delta);
    }

    if (state[SDL_SCANCODE_S]) {
        camera.move_backward(delta);
    }

    if (state[SDL_SCANCODE_A]) {
        camera.move_left(delta);
    }

    if (state[SDL_SCANCODE_D]) {
        camera.move_right(delta);
    }


    camera.speed = state[SDL_SCANCODE_LSHIFT] ? 30.0f : 10.0f;
}


void setup_scripts_system(flecs::entity e, Script& script) {
    if (!script.lua_state) {
        script.lua_state = luaL_newstate();
        luaL_openlibs(script.lua_state);

        lua_getglobal(script.lua_state, "package");
        lua_getfield(script.lua_state, -1, "path"); // get package.path
        std::string path = lua_tostring(script.lua_state, -1); // current paths
        lua_pop(script.lua_state, 1); // pop old path

        path.append(";res/scripts/?.lua"); // add your scripts folder
        lua_pushstring(script.lua_state, path.c_str());
        lua_setfield(script.lua_state, -2, "path"); // package.path = new path
        lua_pop(script.lua_state, 1); // pop package table

        FileAccess lua_file(script.path, ModeFlags::READ);
        const std::string& lua_script = lua_file.get_file_as_str();

        if (luaL_dostring(script.lua_state, lua_script.c_str()) != LUA_OK) {
            const char* err = lua_tostring(script.lua_state, -1);
            LOG_ERROR("Failed to load script %s: %s", script.path.c_str(), err);
            lua_pop(script.lua_state, 1);
            return;
        }

        generate_bindings(script.lua_state);

        push_entity_to_lua(script.lua_state, e);

        // Call ready() if it exists
        lua_getglobal(script.lua_state, "ready");
        if (lua_isfunction(script.lua_state, -1)) {
            if (lua_pcall(script.lua_state, 0, 0, 0) != LUA_OK) {
                const char* err = lua_tostring(script.lua_state, -1);
                LOG_ERROR("Error in ready() of script %s: %s", script.path.c_str(), err);
                lua_pop(script.lua_state, 1);
            } else {
                script.ready_called = true;
            }
        } else {
            lua_pop(script.lua_state, 1); // pop non-function
            LOG_WARN("No `ready` function found in script %s", script.path.c_str());
        }
    }
}

void process_scripts_system(Script& script) {
    if (!script.ready_called) {
        LOG_WARN("Script not initialized properly");
        return;
    }

    lua_getglobal(script.lua_state, "update"); // push global `update` function onto stack

    if (!lua_isfunction(script.lua_state, -1)) {
        lua_pop(script.lua_state, 1); // not a function, remove from stack
        return;
    }

    lua_pushnumber(script.lua_state, static_cast<lua_Number>(GEngine->get_timer().delta)); // push delta time as argument

    // call function with 1 argument, 0 return values
    if (lua_pcall(script.lua_state, 1, 0, 0) != LUA_OK) {
        const char* err_msg = lua_tostring(script.lua_state, -1);
        printf("Error running function `update` in script %s: %s\n", script.path.c_str(), err_msg);
        lua_pop(script.lua_state, 1); // remove error message from stack
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
