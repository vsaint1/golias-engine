#include "core/binding/lua.h"
#include "core/engine.h"

void push_sdl_event_to_lua(lua_State* L, const SDL_Event& event) {
    lua_newtable(L); // Main event table

    lua_pushinteger(L, event.type);
    lua_setfield(L, -2, "type");

    lua_pushinteger(L, event.common.timestamp);
    lua_setfield(L, -2, "timestamp");

    switch (event.type) {
    case SDL_EVENT_FINGER_MOTION:
    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_UP:
        {
            lua_newtable(L); // finger table
            lua_pushnumber(L, event.tfinger.fingerID);
            lua_setfield(L, -2, "fingerid");
            lua_pushnumber(L, event.tfinger.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, event.tfinger.y);
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, event.tfinger.dx);
            lua_setfield(L, -2, "dx");
            lua_pushnumber(L, event.tfinger.dy);
            lua_setfield(L, -2, "dy");
            lua_pushnumber(L, event.tfinger.pressure);
            lua_setfield(L, -2, "pressure");
            lua_pushinteger(L, event.tfinger.windowID);
            lua_setfield(L, -2, "windowid");
            lua_setfield(L, -2, "tfinger");
            break;
        }
    case SDL_EVENT_MOUSE_MOTION:
        {
            lua_newtable(L);
            lua_pushnumber(L, event.motion.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, event.motion.y);
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, event.motion.xrel);
            lua_setfield(L, -2, "xrel");
            lua_pushnumber(L, event.motion.yrel);
            lua_setfield(L, -2, "yrel");
            lua_pushinteger(L, event.motion.which);
            lua_setfield(L, -2, "which");
            lua_pushinteger(L, event.motion.state);
            lua_setfield(L, -2, "state");
            lua_setfield(L, -2, "motion");
            break;
        }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            lua_newtable(L);
            lua_pushinteger(L, event.button.button);
            lua_setfield(L, -2, "button");
            lua_pushboolean(L, event.button.down);
            lua_setfield(L, -2, "down");
            lua_pushinteger(L, event.button.clicks);
            lua_setfield(L, -2, "clicks");
            lua_pushnumber(L, event.button.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, event.button.y);
            lua_setfield(L, -2, "y");
            lua_setfield(L, -2, "button");
            break;
        }

    case SDL_EVENT_MOUSE_WHEEL:
        {
            lua_newtable(L);
            lua_pushnumber(L, event.wheel.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, event.wheel.y);
            lua_setfield(L, -2, "y");
            lua_pushinteger(L, event.wheel.direction);
            lua_setfield(L, -2, "direction");
            lua_pushnumber(L, event.wheel.mouse_x);
            lua_setfield(L, -2, "mouse_x");
            lua_pushnumber(L, event.wheel.mouse_y);
            lua_setfield(L, -2, "mouse_y");
            lua_setfield(L, -2, "wheel");
            break;
        }

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        {
            lua_newtable(L);
            lua_pushinteger(L, event.key.scancode);
            lua_setfield(L, -2, "scancode");
            lua_pushinteger(L, event.key.key);
            lua_setfield(L, -2, "keycode");
            lua_pushinteger(L, event.key.mod);
            lua_setfield(L, -2, "mod");
            lua_pushboolean(L, event.key.down);
            lua_setfield(L, -2, "down");
            lua_pushboolean(L, event.key.repeat);
            lua_setfield(L, -2, "repeat");
            lua_setfield(L, -2, "key");
            break;
        }

    case SDL_EVENT_TEXT_INPUT:
        {
            lua_newtable(L);
            lua_pushstring(L, event.text.text);
            lua_setfield(L, -2, "text");
            lua_setfield(L, -2, "text_input");
            break;
        }

    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_MAXIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        {
            lua_newtable(L); // window table
            lua_pushinteger(L, event.window.windowID);
            lua_setfield(L, -2, "windowid");
            if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                lua_pushinteger(L, event.window.data1);
                lua_setfield(L, -2, "width");
                lua_pushinteger(L, event.window.data2);
                lua_setfield(L, -2, "height");
            }
            lua_setfield(L, -2, "window");
            break;
        }

    case SDL_EVENT_QUIT:
        {
            break;
        }

    default:
        break;
    }
}


void push_input_to_lua(lua_State* L) {
    sol::state_view lua(L);
    
    sol::table input = lua.create_table();
    
    // Return table with x, y fields
    input["get_mouse_position"] = [](sol::this_state s) -> sol::table {
        sol::state_view lua(s);
        glm::vec2 pos = get_mouse_position();
        sol::table result = lua.create_table();
        result["x"] = pos.x;
        result["y"] = pos.y;
        return result;
    };
    
    input["is_key_pressed"] = [](int sdl_key_code) -> bool {
        return is_key_pressed(sdl_key_code);
    };
    
    lua["Input"] = input;
}

void push_camera3d_to_lua(lua_State* L) {
    sol::state_view lua(L);
    
    lua.new_usertype<Camera3D>("Camera3D",
        "yaw", &Camera3D::yaw,
        "pitch", &Camera3D::pitch,
        "fov", &Camera3D::fov,
        "speed", &Camera3D::speed,
        "view_distance", &Camera3D::view_distance,
        
        "get_view_matrix", [](Camera3D& cam, Transform3D& transform) -> glm::mat4 {
            return cam.get_view(transform);
        },
        "get_projection_matrix", [](Camera3D& cam, int width, int height) -> glm::mat4 {
            return cam.get_projection(width, height);
        },
        "move_forward", [](Camera3D& cam, Transform3D& transform, float dt) {
            cam.move_forward(transform, dt);
        },
        "move_backward", [](Camera3D& cam, Transform3D& transform, float dt) {
            cam.move_backward(transform, dt);
        },
        "move_left", [](Camera3D& cam, Transform3D& transform, float dt) {
            cam.move_left(transform, dt);
        },
        "move_right", [](Camera3D& cam, Transform3D& transform, float dt) {
            cam.move_right(transform, dt);
        },
        "look_at", sol::overload(
            [](Camera3D& cam, float xoffset, float yoffset) {
                cam.look_at(xoffset, yoffset, 0.1f);
            },
            [](Camera3D& cam, float xoffset, float yoffset, float sensitivity) {
                cam.look_at(xoffset, yoffset, sensitivity);
            }
        ),
        "zoom", &Camera3D::zoom
    );
}

void generate_bindings(lua_State* L) {
    sol::state_view lua(L);

    // MeshInstance3D
    lua.new_usertype<MeshInstance3D>("MeshInstance3D",
        "size", &MeshInstance3D::size,
        "color", &MeshInstance3D::color
    );
    
    // Transform2D
    lua.new_usertype<Transform2D>("Transform2D",
        "position", &Transform2D::position,
        "scale", &Transform2D::scale,
        "rotation", &Transform2D::rotation
    );

    // Transform3D
    lua.new_usertype<Transform3D>("Transform3D",
        "position", &Transform3D::position,
        "rotation", &Transform3D::rotation,
        "scale", &Transform3D::scale
    );

    // Shape2D
    lua.new_usertype<Shape2D>("Shape2D",
        "color", &Shape2D::color,
        "filled", &Shape2D::filled
    );

    // Label2D
    lua.new_usertype<Label2D>("Label2D",
        "text", &Label2D::text,
        "color", &Label2D::color,
        "font_name", &Label2D::font_name,
        "font_size", &Label2D::font_size
    );

    // vec2
    lua.new_usertype<glm::vec2>("Vector2",
        sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    // vec3
    lua.new_usertype<glm::vec3>("Vector3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    // vec4
    lua.new_usertype<glm::vec4>("Vector4",
        sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(),
        "x", &glm::vec4::x,
        "y", &glm::vec4::y,
        "z", &glm::vec4::z,
        "w", &glm::vec4::w,
        "r", &glm::vec4::r,
        "g", &glm::vec4::g,
        "b", &glm::vec4::b,
        "a", &glm::vec4::a
    );

    push_camera3d_to_lua(L);

    // Engine
    lua.new_usertype<Engine>("Engine",
        "get_config", &Engine::get_config
    );

    // EngineConfig
    lua.new_usertype<EngineConfig>("EngineConfig",
        "get_viewport", &EngineConfig::get_viewport,
        "get_renderer_device", &EngineConfig::get_renderer_device,
        "get_window", &EngineConfig::get_window
    );

    // Viewport
    lua.new_usertype<Viewport>("Viewport",
        "width", &Viewport::width,
        "height", &Viewport::height,
        "scale", &Viewport::scale
    );

    // Window
    lua.new_usertype<Window>("Window",
        "width", &Window::width,
        "height", &Window::height,
        "dpi_scale", &Window::dpi_scale
    );

    // RendererDevice
    lua.new_usertype<RendererDevice>("RendererDevice",
        "backend", &RendererDevice::backend,
        "texture_filtering", &RendererDevice::texture_filtering
    );

    // Push Input table
    push_input_to_lua(L);

    // Global singleton Engine
    lua["Engine"] = GEngine.get();

    // Scene table
    sol::table scene = lua.create_table();
    scene["change_scene"] = [](const char* scene_name) {
        change_scene(scene_name);
    };
    scene["get_entities_count"] = []() -> int {
        return 20;
    };
    lua["Scene"] = scene;
}

void push_entity_table_to_lua(lua_State* L, flecs::entity e);

void push_entity_to_lua(lua_State* L, flecs::entity e) {
    push_entity_table_to_lua(L, e);
    sol::state_view lua(L);
    lua["self"] = sol::stack::pop<sol::table>(lua);
}

void push_entity_table_to_lua(lua_State* L, flecs::entity e) {
    sol::state_view lua(L);
    sol::table entity = lua.create_table();

    entity["id"] = e.id();
    entity["name"] = e.name();

    sol::table meta = lua.create_table();
    meta[sol::meta_function::index] = [](sol::table self, const std::string& key) -> sol::object {
        sol::state_view lua = self.lua_state();
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);

        if (key == "transform" && entity.has<Transform3D>()) {
            return sol::make_object(lua, entity.get_mut<Transform3D>());
        }
        if (key == "transform2d" && entity.has<Transform2D>()) {
            return sol::make_object(lua, entity.get_mut<Transform2D>());
        }
        if (key == "camera" && entity.has<Camera3D>()) {
            return sol::make_object(lua, entity.get_mut<Camera3D>());
        }
        if (key == "shape2d" && entity.has<Shape2D>()) {
            return sol::make_object(lua, entity.get_mut<Shape2D>());
        }
        if (key == "label2d" && entity.has<Label2D>()) {
            return sol::make_object(lua, entity.get_mut<Label2D>());
        }

        return sol::nil;
    };
    entity[sol::metatable_key] = meta;

    entity["move_forward"] = [](sol::table self, float dt) {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        if (entity.has<Camera3D>() && entity.has<Transform3D>()) {
            auto& cam = entity.get_mut<Camera3D>();
            auto& transform = entity.get_mut<Transform3D>();
                cam.move_forward(transform, dt);
        }
    };
    
    entity["move_backward"] = [](sol::table self, float dt) {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        if (entity.has<Camera3D>() && entity.has<Transform3D>()) {
            auto& cam = entity.get_mut<Camera3D>();
            auto& transform = entity.get_mut<Transform3D>();
            cam.move_backward(transform, dt);
        }
    };
    
    entity["move_left"] = [](sol::table self, float dt) {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        if (entity.has<Camera3D>() && entity.has<Transform3D>()) {
            auto& cam = entity.get_mut<Camera3D>();
            auto& transform = entity.get_mut<Transform3D>();
            cam.move_left(transform, dt);

        }
    };
    
    entity["move_right"] = [](sol::table self, float dt) {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        if (entity.has<Camera3D>() && entity.has<Transform3D>()) {
            auto& cam = entity.get_mut<Camera3D>();
            auto& transform = entity.get_mut<Transform3D>();
            cam.move_right(transform, dt);
        }
    };
    
    entity["look_at"] = sol::overload(
        [](sol::table self, float xoffset, float yoffset) {
            flecs::entity_t id = self["id"];
            auto entity = GEngine->get_world().entity(id);
            if (entity.has<Camera3D>()) {
                auto& cam = entity.get_mut<Camera3D>();
                cam.look_at(xoffset, yoffset, 0.1f);
            }
        },
        [](sol::table self, float xoffset, float yoffset, float sensitivity) {
            flecs::entity_t id = self["id"];
            auto entity = GEngine->get_world().entity(id);
            if (entity.has<Camera3D>()) {
                auto& cam = entity.get_mut<Camera3D>();
                cam.look_at(xoffset, yoffset, sensitivity);
            }
        }
    );
    
    entity["zoom"] = [](sol::table self, float yoffset) {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        if (entity.has<Camera3D>()) {
            auto& cam = entity.get_mut<Camera3D>();
            cam.zoom(yoffset);
        }
    };

    // get_node(name: string) -> Entity | nil
    entity["get_node"] = [](sol::table self, const char* name) -> sol::object {
        sol::state_view lua = self.lua_state();
        flecs::entity_t id = self["id"];
        auto parent = GEngine->get_world().entity(id);

        flecs::entity found;
        parent.children([&](flecs::entity child) {
            if (!found.is_valid() && strcmp(child.name(), name) == 0) {
                found = child;
            }
        });

        if (!found.is_valid()) {
            return sol::nil;
        }

        push_entity_table_to_lua(lua.lua_state(), found);
        return sol::stack::pop<sol::object>(lua);
    };

    // get_children() -> Entity[]
    entity["get_children"] = [](sol::table self) -> sol::table {
        sol::state_view lua = self.lua_state();
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);

        sol::table children = lua.create_table();
        int index = 1;
        entity.children([&](flecs::entity child) {
            push_entity_table_to_lua(lua.lua_state(), child);
            children[index++] = sol::stack::pop<sol::table>(lua);
        });

        return children;
    };

    // get_child_count() -> integer
    entity["get_child_count"] = [](sol::table self) -> int {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        int count = 0;
        entity.children([&](flecs::entity) { count++; });
        return count;
    };

    // find_child(name: string, recursive: bool = false, component: string = nil) -> Component | Entity | nil
    entity["find_child"] = [](sol::table self, const char* name, sol::optional<bool> recursive, sol::optional<const char*> component) -> sol::object {
        sol::state_view lua = self.lua_state();
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        flecs::entity result;

        bool is_recursive = recursive.value_or(false);
        std::function<void(flecs::entity)> search;
        search = [&](flecs::entity parent) {
            parent.children([&](flecs::entity child) {
                if (result.is_valid()) return;
                if (strcmp(child.name(), name) == 0) {
                    result = child;
                    return;
                }
                if (is_recursive) {
                    search(child);
                }
            });
        };

        search(entity);

        if (!result.is_valid()) {
            return sol::nil;
        }

        // If component specified, return component directly
        if (component.has_value()) {
            const char* comp = component.value();
            if (strcmp(comp, "Camera3D") == 0 && result.has<Camera3D>()) {
                return sol::make_object(lua, result.get_mut<Camera3D>());
            }
            if (strcmp(comp, "Transform3D") == 0 && result.has<Transform3D>()) {
                return sol::make_object(lua, result.get_mut<Transform3D>());
            }
            if (strcmp(comp, "Transform2D") == 0 && result.has<Transform2D>()) {
                return sol::make_object(lua, result.get_mut<Transform2D>());
            }
            if (strcmp(comp, "Shape2D") == 0 && result.has<Shape2D>()) {
                return sol::make_object(lua, result.get_mut<Shape2D>());
            }
            return sol::nil;
        }

        // Otherwise return entity
        push_entity_table_to_lua(lua.lua_state(), result);
        return sol::stack::pop<sol::object>(lua);
    };

    // has(component_name: string) -> boolean
    entity["has"] = [](sol::table self, const char* component) -> bool {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);

        if (strcmp(component, "Transform3D") == 0) return entity.has<Transform3D>();
        if (strcmp(component, "Transform2D") == 0) return entity.has<Transform2D>();
        if (strcmp(component, "Camera3D") == 0) return entity.has<Camera3D>();
        if (strcmp(component, "MeshInstance3D") == 0) return entity.has<MeshInstance3D>();
        if (strcmp(component, "Shape2D") == 0) return entity.has<Shape2D>();
        if (strcmp(component, "Script") == 0) return entity.has<Script>();

        return false;
    };

    // is_valid() -> boolean
    entity["is_valid"] = [](sol::table self) -> bool {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        return entity.is_valid();
    };

    // queue_free() - mark for deletion
    entity["queue_free"] = [](sol::table self) {
        flecs::entity_t id = self["id"];
        auto entity = GEngine->get_world().entity(id);
        entity.destruct();
    };

    sol::stack::push(L, entity);
}