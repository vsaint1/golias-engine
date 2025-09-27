#include "core/binding/lua.h"

#include "core/engine.h"


void generate_bindings(lua_State* L) {  
    using namespace binding;

    // Create metatables for all types
    create_metatable<flecs::entity>(L);
    create_metatable<Transform2D>(L);
    create_metatable<Shape>(L);
    create_metatable<Label2D>(L);
    create_metatable<glm::vec2>(L);
    create_metatable<glm::vec4>(L);
    create_metatable<EngineConfig>(L);
    create_metatable<Viewport>(L);
    create_metatable<RendererDevice>(L);
    create_metatable<std::string>(L);
    create_metatable<Engine>(L);

    // Transform2D bindings
    add_property<Transform2D>("position", &Transform2D::position);
    add_property<Transform2D>("scale", &Transform2D::scale);
    add_property<Transform2D>("rotation", &Transform2D::rotation);

    // Shape bindings
    add_property<Shape>("color", &Shape::color);
    add_property<Shape>("filled", &Shape::filled);

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
    add_property<glm::vec4>("r", &glm::vec4::r);
    add_property<glm::vec4>("g", &glm::vec4::g);
    add_property<glm::vec4>("b", &glm::vec4::b);
    add_property<glm::vec4>("a", &glm::vec4::a);

    // FIXME: returning garbage, need to fix
    // EngineConfig bindings
    add_method<Engine>("get_config", BINDING_METHOD(Engine, get_config));

    add_method<EngineConfig>("get_viewport", BINDING_METHOD(EngineConfig, get_viewport));
    add_method<EngineConfig>("get_renderer_device", BINDING_METHOD(EngineConfig, get_renderer_device));

    // Viewport bindings
    add_property<Viewport>("width", &Viewport::width);
    add_property<Viewport>("height", &Viewport::height);
    add_property<Viewport>("scale", &Viewport::scale);

    // RendererDevice bindings
    add_property<RendererDevice>("backend", &RendererDevice::backend);
    add_property<RendererDevice>("texture_filtering", &RendererDevice::texture_filtering);
   

    // ===================================
    // GLOBAL SINGLETONS - Push once during binding generation
    // ===================================
    
    // Push EngineConfig as global singleton
    push_userdata(L, GEngine.get());
    lua_setglobal(L, "Engine");  
    
    
    // Scene table (also global)
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

    if (e.has<Shape>()) {
        binding::push_userdata(L, &e.get_mut<Shape>());
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
