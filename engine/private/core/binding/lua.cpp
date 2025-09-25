#include "core/binding/lua.h"



// =======================================================
// Lua push value specializations
// =======================================================
template <>
void lua_push_value<float>(lua_State* L, const float& value) {
    lua_pushnumber(L, static_cast<double>(value));
}

template <>
void lua_push_value<double>(lua_State* L, const double& value) {
    lua_pushnumber(L, value);
}

template <>
void lua_push_value<int>(lua_State* L, const int& value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

template <>
void lua_push_value<bool>(lua_State* L, const bool& value) {
    lua_pushboolean(L, value);
}

template <>
void lua_push_value<std::string>(lua_State* L, const std::string& value) {
    lua_pushstring(L, value.c_str());
}

template <>
void lua_push_value<glm::vec2>(lua_State* L, const glm::vec2& value) {
    lua_newtable(L);
    lua_pushnumber(L, value.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, value.y);
    lua_setfield(L, -2, "y");
}

template <>
void lua_push_value<glm::vec4>(lua_State* L, const glm::vec4& value) {
    lua_newtable(L);
    lua_pushnumber(L, value.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, value.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, value.z); lua_setfield(L, -2, "z");
    lua_pushnumber(L, value.w); lua_setfield(L, -2, "w");
    // alias as rgba
    lua_pushnumber(L, value.x); lua_setfield(L, -2, "r");
    lua_pushnumber(L, value.y); lua_setfield(L, -2, "g");
    lua_pushnumber(L, value.z); lua_setfield(L, -2, "b");
    lua_pushnumber(L, value.w); lua_setfield(L, -2, "a");
}

// =======================================================
// Lua get value specializations
// =======================================================
template <>
void lua_get_value<float>(lua_State* L, int index, float& value) {
    value = static_cast<float>(luaL_checknumber(L, index));
}

template <>
void lua_get_value<double>(lua_State* L, int index, double& value) {
    value = luaL_checknumber(L, index);
}

template <>
void lua_get_value<int>(lua_State* L, int index, int& value) {
    value = static_cast<int>(luaL_checkinteger(L, index));
}

template <>
void lua_get_value<bool>(lua_State* L, int index, bool& value) {
    value = lua_toboolean(L, index) != 0;
}

template <>
void lua_get_value<std::string>(lua_State* L, int index, std::string& value) {
    if (lua_isstring(L, index)) {
        value = lua_tostring(L, index);
    }
}

template <>
void lua_get_value<glm::vec2>(lua_State* L, int index, glm::vec2& value) {
    lua_getfield(L, index, "x"); if (lua_isnumber(L, -1)) value.x = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, index, "y"); if (lua_isnumber(L, -1)) value.y = lua_tonumber(L, -1); lua_pop(L, 1);
}

template <>
void lua_get_value<glm::vec4>(lua_State* L, int index, glm::vec4& value) {
    lua_getfield(L, index, "x"); if (lua_isnumber(L, -1)) value.x = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, index, "y"); if (lua_isnumber(L, -1)) value.y = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, index, "z"); if (lua_isnumber(L, -1)) value.z = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, index, "w"); if (lua_isnumber(L, -1)) value.w = lua_tonumber(L, -1); lua_pop(L, 1);
}

// =======================================================
// Getter
// =======================================================
template <typename T>
int lua_get_field(lua_State* L, T& comp) {
    const char* key = luaL_checkstring(L, 2);
    bool found = false;

    std::apply([&](auto&&... pair) {
        ((found || (std::string(pair.first) == key ? (lua_push_value(L, comp.*(pair.second)), found = true) : false)) || ...);
    }, Binding<T>::fields());

    if (!found) {
        lua_pushnil(L);
    }
    return 1;
}

// =======================================================
// Setter
// =======================================================
template <typename T>
int lua_set_field(lua_State* L, T& comp) {
    const char* key = luaL_checkstring(L, 2);
    bool found = false;

    std::apply([&](auto&&... pair) {
        ((found || (std::string(pair.first) == key ? (lua_get_value(L, 3, comp.*(pair.second)), found = true) : false)) || ...);
    }, Binding<T>::fields());

    if (!found) {
        LOG_WARN("Attempted to set unknown field '%s' on component", key);
    }
    return 0;
}

// =======================================================
// Push component to Lua
// =======================================================
template <typename T>
void push_component_to_lua(lua_State* L, flecs::entity& e) {
    T& comp_ref = e.get_mut<T>();
    void** ud = static_cast<void**>(lua_newuserdata(L, sizeof(void*)));
    *ud = &comp_ref;

    std::string mt_name = typeid(T).name();
    if (luaL_newmetatable(L, mt_name.c_str())) {
        // __index
        lua_pushstring(L, "__index");
        lua_pushlightuserdata(L, &comp_ref);
        lua_pushcclosure(L, [](lua_State* L) -> int {
            T* comp = static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1)));
            return lua_get_field(L, *comp);
        }, 1);
        lua_settable(L, -3);

        // __newindex
        lua_pushstring(L, "__newindex");
        lua_pushlightuserdata(L, &comp_ref);
        lua_pushcclosure(L, [](lua_State* L) -> int {
            T* comp = static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1)));
            return lua_set_field(L, *comp);
        }, 1);
        lua_settable(L, -3);
    }
    lua_setmetatable(L, -2);
}

// =======================================================
// Push engine functions under "Scene" namespace
// =======================================================
void push_engine_functions_to_lua(lua_State* L) {
    lua_newtable(L);

    lua_pushstring(L, "change_scene");
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* scene_name = luaL_checkstring(L, 1);
        change_scene(scene_name);
        return 0;
    });
    lua_settable(L, -3);

    lua_setglobal(L, "Scene");
}

// =======================================================
// Generate entity bindings
// =======================================================
void generate_bindings_to_lua(lua_State* L, flecs::world& world, flecs::entity& e) {
    lua_newtable(L);

    lua_pushstring(L, "id");
    lua_pushinteger(L, static_cast<lua_Integer>(e.id()));
    lua_settable(L, -3);

    lua_pushstring(L, "name");
    lua_pushstring(L, e.name().c_str());
    lua_settable(L, -3);

    e.each([L, &world, &e](flecs::id comp_id) {
        flecs::entity comp = world.entity(comp_id);
        lua_pushstring(L, comp.name().c_str());

        if (comp == world.component<Transform2D>()) {
            push_component_to_lua<Transform2D>(L, e);
        } else if (comp == world.component<Shape>()) {
            push_component_to_lua<Shape>(L, e);
        } else if (comp == world.component<Label2D>()) {
            push_component_to_lua<Label2D>(L, e);
        } else {
            lua_pushnil(L);
        }
        lua_settable(L, -3);
    });

    lua_setglobal(L, "self");

    // push engine API (Scene.change_scene, etc.)
    push_engine_functions_to_lua(L);
}


