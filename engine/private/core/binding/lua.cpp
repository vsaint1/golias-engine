#include "core/binding/lua.h"

#include "core/engine.h"

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

void generate_bindings(lua_State* L) {
    using namespace binding;

    // Create metatables for all types
    create_metatable<flecs::entity>(L);
    create_metatable<Transform2D>(L);
    create_metatable<Shape2D>(L);
    create_metatable<Label2D>(L);
    create_metatable<glm::vec2>(L);
    create_metatable<glm::vec4>(L);
    create_metatable<EngineConfig>(L);
    create_metatable<Viewport>(L);
    create_metatable<Window>(L);
    create_metatable<RendererDevice>(L);
    create_metatable<std::string>(L);
    create_metatable<Engine>(L);

    push_input_to_lua(L);

    // Transform2D bindings
    add_property<Transform2D>("position", &Transform2D::position);
    add_property<Transform2D>("scale", &Transform2D::scale);
    add_property<Transform2D>("rotation", &Transform2D::rotation);

    // Shape bindings
    add_property<Shape2D>("color", &Shape2D::color);
    add_property<Shape2D>("filled", &Shape2D::filled);

    // Label2D bindings
    add_property<Label2D>("text", &Label2D::text);
    add_property<Label2D>("color", &Label2D::color);
    add_property<Label2D>("font_name", &Label2D::font_name);
    add_property<Label2D>("font_size", &Label2D::font_size);

    // vec2 bindings
    add_property<glm::vec2>("x", &glm::vec2::x);
    add_property<glm::vec2>("y", &glm::vec2::y);

    // vec4 bindings
    add_property<glm::vec4>("x", &glm::vec4::x);
    add_property<glm::vec4>("y", &glm::vec4::y);
    add_property<glm::vec4>("z", &glm::vec4::z);
    add_property<glm::vec4>("w", &glm::vec4::w);

    // vec4 color aliases
    add_property<glm::vec4>("r", &glm::vec4::r);
    add_property<glm::vec4>("g", &glm::vec4::g);
    add_property<glm::vec4>("b", &glm::vec4::b);
    add_property<glm::vec4>("a", &glm::vec4::a);

    // EngineConfig / Viewport / Window / RendererDevice methods as lambdas
    add_method<Engine>("get_config", [](lua_State* L) -> int {
        Engine* engine = binding::check_userdata<Engine>(L, 1);
        binding::push_userdata(L, &engine->get_config());
        return 1;
    });

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

    // Viewport bindings
    add_property<Viewport>("width", &Viewport::width);
    add_property<Viewport>("height", &Viewport::height);
    add_property<Viewport>("scale", &Viewport::scale);

    // Window bindings
    add_property<Window>("width", &Window::width);
    add_property<Window>("height", &Window::height);
    add_property<Window>("dpi_scale", &Window::dpi_scale);

    // RendererDevice bindings
    add_property<RendererDevice>("backend", &RendererDevice::backend);
    add_property<RendererDevice>("texture_filtering", &RendererDevice::texture_filtering);

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


void push_entity_to_lua(lua_State* L, flecs::entity e) {
    lua_newtable(L); // Create the 'self' table

    // -------------------------
    // entity metadata
    // -------------------------
    lua_pushinteger(L, e.id());
    lua_setfield(L, -2, "id");

    lua_pushstring(L, e.name());
    lua_setfield(L, -2, "name");

    lua_pushboolean(L, e.is_valid());
    lua_setfield(L, -2, "is_valid");

    // TODO: implement these functions properly
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_pushboolean(L, true);
        return 1;
    });
    lua_setfield(L, -2, "has_component");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_pushboolean(L, true);
        return 1;
    });
    lua_setfield(L, -2, "add_component");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_pushnil(L);
        return 1;
    });
    lua_setfield(L, -2, "get_component");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_pushboolean(L, true);
        return 1;
    });
    lua_setfield(L, -2, "remove_component");

    // -------------------------
    // components (direct members)
    // -------------------------
    if (e.has<Transform2D>()) {
        binding::push_userdata(L, &e.get_mut<Transform2D>());
        lua_setfield(L, -2, "transform");
    }

    if (e.has<Shape2D>()) {
        binding::push_userdata(L, &e.get_mut<Shape2D>());
        lua_setfield(L, -2, "shape");
    }

    if (e.has<Label2D>()) {
        binding::push_userdata(L, &e.get_mut<Label2D>());
        lua_setfield(L, -2, "label");
    }

    // -------------------------
    // expose the entity to Lua as `self`
    // scripts can just use `self` directly
    // -------------------------
    lua_setglobal(L, "self");
}
