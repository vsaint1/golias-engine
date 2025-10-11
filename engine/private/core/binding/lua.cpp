#include "core/binding/lua.h"

#include "core/engine.h"

using namespace binding;


// Push SDL_Event to Lua as a table
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
            lua_setfield(L, -2, "finger_id");
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
            lua_setfield(L, -2, "window_id");
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
            lua_setfield(L, -2, "window_id");
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
    lua_newtable(L);

    lua_pushcfunction(L, [](lua_State* L) -> int {
        glm::vec2 pos = get_mouse_position();

        lua_newtable(L);
        lua_pushnumber(L, pos.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, pos.y);
        lua_setfield(L, -2, "y");

        return 1;
    });
    lua_setfield(L, -2, "get_mouse_position");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        int sdl_key_code = static_cast<int>(luaL_checkinteger(L, 1));
        bool pressed     = is_key_pressed(sdl_key_code);
        lua_pushboolean(L, pressed);
        return 1;
    });
    lua_setfield(L, -2, "is_key_pressed");

    lua_setglobal(L, "Input");
}

void push_camera3d_to_lua(lua_State* L) {
    create_metatable<Camera3D>(L);

    // Properties
    add_property<Camera3D>("position", &Camera3D::position);
    add_property<Camera3D>("yaw", &Camera3D::yaw);
    add_property<Camera3D>("pitch", &Camera3D::pitch);
    add_property<Camera3D>("fov", &Camera3D::fov);
    add_property<Camera3D>("speed", &Camera3D::speed);
    add_property<Camera3D>("view_distance", &Camera3D::view_distance);

    // Methods
    add_method<Camera3D>("get_view_matrix", [](lua_State* L) -> int {
        Camera3D* cam  = binding::check_userdata<Camera3D>(L, 1);
        glm::mat4 view = cam->get_view();
        binding::push_userdata(L, &view);
        return 1;
    });

    add_method<Camera3D>("get_projection_matrix", [](lua_State* L) -> int {
        Camera3D* cam  = binding::check_userdata<Camera3D>(L, 1);
        int width      = static_cast<int>(luaL_checkinteger(L, 2));
        int height     = static_cast<int>(luaL_checkinteger(L, 3));
        glm::mat4 proj = cam->get_projection(width, height);
        binding::push_userdata(L, &proj);
        return 1;
    });

    add_method<Camera3D>("move_forward", [](lua_State* L) -> int {
        Camera3D* cam = binding::check_userdata<Camera3D>(L, 1);
        float dt      = static_cast<float>(luaL_checknumber(L, 2));
        cam->move_forward(dt);
        return 0;
    });

    add_method<Camera3D>("move_backward", [](lua_State* L) -> int {
        Camera3D* cam = binding::check_userdata<Camera3D>(L, 1);
        float dt      = static_cast<float>(luaL_checknumber(L, 2));
        cam->move_backward(dt);
        return 0;
    });

    add_method<Camera3D>("move_left", [](lua_State* L) -> int {
        Camera3D* cam = binding::check_userdata<Camera3D>(L, 1);
        float dt      = static_cast<float>(luaL_checknumber(L, 2));
        cam->move_left(dt);
        return 0;
    });

    add_method<Camera3D>("move_right", [](lua_State* L) -> int {
        Camera3D* cam = binding::check_userdata<Camera3D>(L, 1);
        float dt      = static_cast<float>(luaL_checknumber(L, 2));
        cam->move_right(dt);
        return 0;
    });

    add_method<Camera3D>("look_at", [](lua_State* L) -> int {
        Camera3D* cam     = binding::check_userdata<Camera3D>(L, 1);
        float xoffset     = static_cast<float>(luaL_checknumber(L, 2));
        float yoffset     = static_cast<float>(luaL_checknumber(L, 3));
        float sensitivity = 0.1f;
        if (lua_gettop(L) >= 4) {
            sensitivity = static_cast<float>(luaL_checknumber(L, 4));
        }
        cam->look_at(xoffset, yoffset, sensitivity);
        return 0;
    });

    add_method<Camera3D>("zoom", [](lua_State* L) -> int {
        Camera3D* cam = binding::check_userdata<Camera3D>(L, 1);
        float yoffset = static_cast<float>(luaL_checknumber(L, 2));
        cam->zoom(yoffset);
        return 0;
    });
}


// Create and populate metatables for all types
void generate_bindings(lua_State* L) {


    // Transform2D
    create_metatable<Transform2D>(L);
    add_property<Transform2D>("position", &Transform2D::position);
    add_property<Transform2D>("scale", &Transform2D::scale);
    add_property<Transform2D>("rotation", &Transform2D::rotation);

    // Transform3D
    create_metatable<Transform3D>(L);
    add_property<Transform3D>("position", &Transform3D::position);
    add_property<Transform3D>("rotation", &Transform3D::rotation);
    add_property<Transform3D>("scale", &Transform3D::scale);

    // Shape2D
    create_metatable<Shape2D>(L);
    add_property<Shape2D>("color", &Shape2D::color);
    add_property<Shape2D>("filled", &Shape2D::filled);

    // Label2D
    create_metatable<Label2D>(L);
    add_property<Label2D>("text", &Label2D::text);
    add_property<Label2D>("color", &Label2D::color);
    add_property<Label2D>("font_name", &Label2D::font_name);
    add_property<Label2D>("font_size", &Label2D::font_size);

    // vec2
    create_metatable<glm::vec2>(L);
    add_property<glm::vec2>("x", &glm::vec2::x);
    add_property<glm::vec2>("y", &glm::vec2::y);

    // vec3
    create_metatable<glm::vec3>(L);
    add_property<glm::vec3>("x", &glm::vec3::x);
    add_property<glm::vec3>("y", &glm::vec3::y);
    add_property<glm::vec3>("z", &glm::vec3::z);

    // vec4
    create_metatable<glm::vec4>(L);
    add_property<glm::vec4>("x", &glm::vec4::x);
    add_property<glm::vec4>("y", &glm::vec4::y);
    add_property<glm::vec4>("z", &glm::vec4::z);
    add_property<glm::vec4>("w", &glm::vec4::w);
    // vec4 color aliases
    add_property<glm::vec4>("r", &glm::vec4::r);
    add_property<glm::vec4>("g", &glm::vec4::g);
    add_property<glm::vec4>("b", &glm::vec4::b);
    add_property<glm::vec4>("a", &glm::vec4::a);


    push_camera3d_to_lua(L);

    // Engine
    create_metatable<Engine>(L);
    add_method<Engine>("get_config", [](lua_State* L) -> int {
        Engine* engine = binding::check_userdata<Engine>(L, 1);
        binding::push_userdata(L, &engine->get_config());
        return 1;
    });

    // EngineConfig
    create_metatable<EngineConfig>(L);
    add_method<EngineConfig>("get_viewport", [](lua_State* L) -> int {
        EngineConfig* config = binding::check_userdata<EngineConfig>(L, 1);
        binding::push_userdata(L, &config->get_viewport());
        return 1;
    });
    add_method<EngineConfig>("get_renderer_device", [](lua_State* L) -> int {
        EngineConfig* config = binding::check_userdata<EngineConfig>(L, 1);
        binding::push_userdata(L, &config->get_renderer_device());
        return 1;
    });
    add_method<EngineConfig>("get_window", [](lua_State* L) -> int {
        EngineConfig* config = binding::check_userdata<EngineConfig>(L, 1);
        binding::push_userdata(L, &config->get_window());
        return 1;
    });

    // Viewport
    create_metatable<Viewport>(L);
    add_property<Viewport>("width", &Viewport::width);
    add_property<Viewport>("height", &Viewport::height);
    add_property<Viewport>("scale", &Viewport::scale);

    // Window
    create_metatable<Window>(L);
    add_property<Window>("width", &Window::width);
    add_property<Window>("height", &Window::height);
    add_property<Window>("dpi_scale", &Window::dpi_scale);

    // RendererDevice
    create_metatable<RendererDevice>(L);
    add_property<RendererDevice>("backend", &RendererDevice::backend);
    add_property<RendererDevice>("texture_filtering", &RendererDevice::texture_filtering);

    // Other types that need metatables but no properties/methods yet
    create_metatable<flecs::entity>(L);
    create_metatable<std::string>(L);

    // Push Input table
    push_input_to_lua(L);

    // Global singleton Engine
    push_userdata(L, GEngine.get());
    lua_setglobal(L, "Engine");

    // Scene table
    lua_newtable(L);
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* scene_name = luaL_checkstring(L, 1);
        change_scene(scene_name);
        return 0;
    });
    lua_setfield(L, -2, "change_scene");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_pushinteger(L, 20);
        return 1;
    });
    lua_setfield(L, -2, "get_entities_count");

    lua_setglobal(L, "Scene");
}

void push_entity_table_to_lua(lua_State* L, flecs::entity e);

void push_entity_to_lua(lua_State* L, flecs::entity e) {
    push_entity_table_to_lua(L, e);
    lua_setglobal(L, "self");
}

void push_entity_table_to_lua(lua_State* L, flecs::entity e) {
    lua_newtable(L); // Create the entity table

    // Store entity ID for methods to use
    lua_pushinteger(L, e.id());
    lua_setfield(L, -2, "_id");

    lua_pushstring(L, e.name());
    lua_setfield(L, -2, "name");

    // -------------------------
    // Create a metatable with __index for dynamic component access
    // -------------------------
    lua_newtable(L); // metatable

    // __index metamethod for dynamic property access
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        const char* key = luaL_checkstring(L, 2);
        auto entity     = GEngine->get_world().entity(id);

        // Check for components
        if (strcmp(key, "transform") == 0 && entity.has<Transform3D>()) {
            binding::push_userdata(L, &entity.get_mut<Transform3D>());
            return 1;
        }
        if (strcmp(key, "transform2d") == 0 && entity.has<Transform2D>()) {
            binding::push_userdata(L, &entity.get_mut<Transform2D>());
            return 1;
        }
        if (strcmp(key, "camera") == 0 && entity.has<Camera3D>()) {
            binding::push_userdata(L, &entity.get_mut<Camera3D>());
            return 1;
        }
        if (strcmp(key, "shape2d") == 0 && entity.has<Shape2D>()) {
            binding::push_userdata(L, &entity.get_mut<Shape2D>());
            return 1;
        }
        if (strcmp(key, "label2d") == 0 && entity.has<Label2D>()) {
            binding::push_userdata(L, &entity.get_mut<Label2D>());
            return 1;
        }

        return 0; // property not found
    });
    lua_setfield(L, -2, "__index");

    lua_setmetatable(L, -2);



lua_pushcfunction(L, [](lua_State* L) -> int {
    lua_getfield(L, 1, "_id");
    flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    const char* name = luaL_checkstring(L, 2);
    auto parent = GEngine->get_world().entity(id);

    flecs::entity found;
    parent.children([&](flecs::entity child) {
        if (!found.is_valid() && strcmp(child.name(), name) == 0) {
            found = child;
        }
    });

    if (!found.is_valid()) {
        lua_pushnil(L);
        return 1;
    }

    push_entity_table_to_lua(L, found);
    return 1;
});
lua_setfield(L, -2, "get_node");

    // get_children() -> Entity[]
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        auto entity = GEngine->get_world().entity(id);

        lua_newtable(L);
        int index = 1;
        entity.children([&](flecs::entity child) {
            push_entity_table_to_lua(L, child);
            lua_rawseti(L, -2, index++);
        });

        return 1;
    });
    lua_setfield(L, -2, "get_children");

    // get_child_count() -> integer
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        auto entity = GEngine->get_world().entity(id);
        int count   = 0;
        entity.children([&](flecs::entity) { count++; });

        lua_pushinteger(L, count);
        return 1;
    });
    lua_setfield(L, -2, "get_child_count");

    // find_child(name: string, recursive: bool = false, component: string = nil) -> Component | Entity | nil
    // If component is specified, returns the component directly, otherwise returns entity
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        const char* name      = luaL_checkstring(L, 2);
        bool recursive        = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;
        const char* component = lua_isstring(L, 4) ? lua_tostring(L, 4) : nullptr;

        auto entity = GEngine->get_world().entity(id);
        flecs::entity result;

        std::function<void(flecs::entity)> search;
        search = [&](flecs::entity parent) {
            parent.children([&](flecs::entity child) {
                if (result.is_valid()) {
                    return;
                }
                if (strcmp(child.name(), name) == 0) {
                    result = child;
                    return;
                }
                if (recursive) {
                    search(child);
                }
            });
        };

        search(entity);

        if (!result.is_valid()) {
            lua_pushnil(L);
            return 1;
        }

        // If component specified, return component directly
        if (component != nullptr) {
            if (strcmp(component, "Camera3D") == 0 && result.has<Camera3D>()) {
                binding::push_userdata(L, &result.get_mut<Camera3D>());
                return 1;
            }
            if (strcmp(component, "Transform3D") == 0 && result.has<Transform3D>()) {
                binding::push_userdata(L, &result.get_mut<Transform3D>());
                return 1;
            }
            if (strcmp(component, "Transform2D") == 0 && result.has<Transform2D>()) {
                binding::push_userdata(L, &result.get_mut<Transform2D>());
                return 1;
            }
            if (strcmp(component, "Shape2D") == 0 && result.has<Shape2D>()) {
                binding::push_userdata(L, &result.get_mut<Shape2D>());
                return 1;
            }
            lua_pushnil(L); // Component not found
            return 1;
        }

        // Otherwise return entity
        push_entity_table_to_lua(L, result);
        return 1;
    });
    lua_setfield(L, -2, "find_child");

    // has(component_name: string) -> boolean
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        const char* component = luaL_checkstring(L, 2);
        auto entity           = GEngine->get_world().entity(id);

        bool has = false;
        if (strcmp(component, "Transform3D") == 0) {
            has = entity.has<Transform3D>();
        } else if (strcmp(component, "Transform2D") == 0) {
            has = entity.has<Transform2D>();
        } else if (strcmp(component, "Camera3D") == 0) {
            has = entity.has<Camera3D>();
        } else if (strcmp(component, "Cube") == 0) {
            has = entity.has<Cube>();
        } else if (strcmp(component, "Shape2D") == 0) {
            has = entity.has<Shape2D>();
        } else if (strcmp(component, "Script") == 0) {
            has = entity.has<Script>();
        }

        lua_pushboolean(L, has);
        return 1;
    });
    lua_setfield(L, -2, "has");

    // is_valid() -> boolean
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        auto entity = GEngine->get_world().entity(id);
        lua_pushboolean(L, entity.is_valid());
        return 1;
    });
    lua_setfield(L, -2, "is_valid");

    // queue_free() - mark for deletion
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_getfield(L, 1, "_id");
        flecs::entity_t id = static_cast<flecs::entity_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        auto entity = GEngine->get_world().entity(id);
        entity.destruct();

        return 0;
    });
    lua_setfield(L, -2, "queue_free");
}
