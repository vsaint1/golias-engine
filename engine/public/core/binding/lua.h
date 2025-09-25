#pragma once

#include "core/component/components.h"
#include "core/api/engine_api.h"



// =======================================================
// BINDING MACROS
// =======================================================
template <typename T>
struct Binding;

#define Field(ComponentType, member) std::make_pair(#member, &ComponentType::member)

#define LUA_GENERATE_BINDING(ComponentType, ...) \
    template <>                                  \
    struct Binding<ComponentType> {              \
        static auto fields() {                   \
            return std::make_tuple(__VA_ARGS__); \
        }                                        \
    };

// Component bindings
LUA_GENERATE_BINDING(Transform2D, Field(Transform2D, position), Field(Transform2D, scale), Field(Transform2D, rotation))
LUA_GENERATE_BINDING(Shape, Field(Shape, color), Field(Shape, filled))
LUA_GENERATE_BINDING(Label2D, Field(Label2D, text), Field(Label2D, offset), Field(Label2D, color), Field(Label2D, font_name),
                     Field(Label2D, font_size))

// =======================================================
// Lua <-> C++ value conversions
// =======================================================
template <typename T>
void lua_push_value(lua_State* L, const T& value);

template <typename T>
void lua_get_value(lua_State* L, int index, T& value);

// =======================================================
// Getter / Setter helpers
// =======================================================
template <typename T>
int lua_get_field(lua_State* L, T& comp);

template <typename T>
int lua_set_field(lua_State* L, T& comp);

// =======================================================
// Push Flecs component to Lua
// =======================================================
template <typename T>
void push_component_to_lua(lua_State* L, flecs::entity& e);

// =======================================================
// Engine API exposure
// =======================================================
void push_engine_functions_to_lua(lua_State* L);

// =======================================================
// Generate Lua bindings for an entity
// =======================================================
void generate_bindings_to_lua(lua_State* L, flecs::world& world, flecs::entity& e);