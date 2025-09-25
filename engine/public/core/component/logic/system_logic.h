#pragma once

#include "core/component/components.h"


void load_scripts_system(Script& script);

void process_scripts_system(Script& script);


void render_primitives_system(Transform2D& t, Shape& s);

void render_labels_system(Transform2D& t, Label2D& l);



// ---------------- LUA PUSH COMPONENT ----------------
template <typename T>
inline void serialize_component_to_lua(lua_State* L, T& comp, const char* global_name) {
    lua_newtable(L); // component table

    lua_newtable(L); // metatable

    // __index (getter)
    lua_pushlightuserdata(L, (void*) &comp);
    lua_pushcclosure(
        L,
        [](lua_State* L) -> int {
            T* c = (T*) lua_touserdata(L, lua_upvalueindex(1));
            return lua_get_field(L, *c);
        },
        1);
    lua_setfield(L, -2, "__index");

    // __newindex (setter)
    lua_pushlightuserdata(L, (void*) &comp);
    lua_pushcclosure(
        L,
        [](lua_State* L) -> int {
            T* c = (T*) lua_touserdata(L, lua_upvalueindex(1));
            return lua_set_field(L, *c);
        },
        1);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);
    lua_setglobal(L, global_name);
}




inline void serialize_entity_to_lua(lua_State* L, flecs::entity e) {
    lua_newtable(L); // self table

    lua_pushstring(L, "id");
    lua_pushinteger(L, (lua_Integer) e.id());
    lua_settable(L, -3);

    lua_pushstring(L, "name");
    lua_pushstring(L, e.name().c_str());
    lua_settable(L, -3);

    lua_setglobal(L, "self");
}
