#include "core/component/logic/system_logic.h"

#include "core/binding/lua.h"
#include "core/engine.h"

#if defined(EMBER_2D)


int sort_by_z_index(flecs::entity_t e1, const Transform2D* t1, flecs::entity_t e2, const Transform2D* t2) {
    (void) e1;
    (void) e2;
    return t1->z_index - t2->z_index;
}

void render_world_2d_system(flecs::entity e, Camera2D& camera) {
    if (!e.is_valid() || !e.has<tags::MainCamera>()) {
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

#endif

#if defined(EMBER_3D)

void render_world_3d_system(flecs::entity e, Camera3D& camera) {


    if (!e.is_valid() || !e.has<tags::MainCamera>()) {
        return;
    }

    const auto& window = GEngine->get_config().get_window();

    // Render all 3D models in the scene
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, const Model& model) {
        
      
       auto  m = GEngine->get_renderer()->load_model(model.path.c_str());
      
        GEngine->get_renderer()->draw_model(t, m.get());
    });

    // Render all cubes in the scene
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, const Cube& cube) { GEngine->get_renderer()->draw_cube(t, cube); });


    // Flush
    GEngine->get_renderer()->flush(camera.get_view(), camera.get_projection(window.width, window.height));
}

#endif

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
    if (luaL_loadstring(script.lua_state, lua_script.c_str()) || 
        lua_pcall(script.lua_state, 0, 0, 0)) {
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

void process_event_scripts_system(Script& script, const SDL_Event& event) {
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
