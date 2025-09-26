#include "core/component/logic/system_logic.h"

#include "core/engine.h"

#include "core/binding/lua.h"


void render_primitives_system(Transform2D& t, Shape& s) {
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



 void setup_scripts_system(flecs::entity e, Script& script) {
    if (!script.lua_state) {
        // create Lua state
        script.lua_state = luaL_newstate();
        luaL_openlibs(script.lua_state);

        // load the Lua file
        FileAccess lua_file(script.path, ModeFlags::READ);
        const std::string& lua_script = lua_file.get_file_as_str();

        if (luaL_dostring(script.lua_state, lua_script.c_str()) != LUA_OK) {
            const char* err = lua_tostring(script.lua_state, -1);
            LOG_ERROR("Failed to load script %s: %s", script.path.c_str(), err);
            lua_pop(script.lua_state, 1);
            return;
        }

        // create a sol::state_view to access the script
        sol::state_view lua(script.lua_state);

        // generate engine bindings
        generate_bindings(lua);

        push_entity_to_lua(lua, e);

        // call `ready` if it exists
        sol::object ready_obj = lua["ready"];
        if (ready_obj.is<sol::function>()) {
            sol::function ready_func = ready_obj.as<sol::function>();
            sol::protected_function_result ready_result = ready_func();
            if (!ready_result.valid()) {
                sol::error err = ready_result;
                LOG_ERROR("Error running function `ready` in script %s: %s", script.path.c_str(), err.what());
                return;
            }

            script.ready_called = true;
        }
    }
}

void process_scripts_system(Script& script) {
    if (!script.ready_called) {
        LOG_WARN("Script not initialized properly");
        return;
    }

    sol::state_view lua(script.lua_state);

    sol::object update_obj = lua["update"];
    if (update_obj.is<sol::function>()) {
        sol::function update_func                    = update_obj.as<sol::function>();
        sol::protected_function_result update_result = update_func(GEngine->get_timer().delta);
        if (!update_result.valid()) {
            sol::error err = update_result;
            LOG_ERROR("Error running function `update` in script %s: %s", script.path.c_str(), err.what());
            return;
        }
    }
}


void render_labels_system(Transform2D& t, Label2D& l) {
    Transform2D temp = t;
    temp.position += l.offset;

    GEngine->get_renderer()->draw_text(temp, l.color, l.font_name.c_str(), "%s", l.text.c_str());
}


void scene_manager_system(flecs::world& world) {
    world.observer<SceneChangeRequest>("SceneChangeRequest_Observer")
        .event(flecs::OnSet)
        .each([&](flecs::iter& it, size_t i, SceneChangeRequest& req) {
            LOG_INFO("Scene requested: %s", req.name.c_str());

            auto new_scene = world.lookup(req.name.c_str());

            if (new_scene.is_valid() && new_scene.has<Scene>()) {

                world.each([&](flecs::entity e, Scene) {
                    e.add(flecs::Disabled);
                    e.remove<ActiveScene>();
                });

                new_scene.remove(flecs::Disabled);
                new_scene.add<ActiveScene>();
                LOG_INFO("Switched to scene: %s", req.name.c_str());
            } else {
                LOG_WARN("Scene '%s' not found", req.name.c_str());
            }

            it.entity(i).remove<SceneChangeRequest>();
        });
}
