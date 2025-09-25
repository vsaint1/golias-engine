#pragma once

#include "core/system/logging.h"


// ==============================================================
// REFLECTION UTILITIES FOR COMPONENTS `RUDIMENTARY`            |
// USAGE EXAMPLE:                                               |
// REFLECT_COMPONENT(Hello, FIELD(Hello, field));               |
// ==============================================================
template <typename T>
struct Reflected; // primary template left undefined

// FIELD macro must be defined outside
#define FIELD(Type, name) std::make_pair(#name, &Type::name)

// REFLECT_COMPONENT generates template specialization
#define REFLECT_COMPONENT(Type, ...)             \
    template <>                                  \
    struct Reflected<Type> {                     \
        static auto fields() {                   \
            return std::make_tuple(__VA_ARGS__); \
        }                                        \
    };



template <typename T>
inline  void lua_push_value(lua_State* L, const T& value) {
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
        lua_pushnumber(L, static_cast<double>(value));
    } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool>) {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    } else if constexpr (std::is_enum_v<T>) {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
        lua_newtable(L);
        lua_pushnumber(L, value.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, value.y);
        lua_setfield(L, -2, "y");

        // Add metatable to handle setting x,y from Lua
        lua_newtable(L); // metatable
        lua_pushlightuserdata(L, (void*) &value);
        lua_pushcclosure(
            L,
            [](lua_State* L) -> int {
                glm::vec2* vec  = (glm::vec2*) lua_touserdata(L, lua_upvalueindex(1));
                const char* key = luaL_checkstring(L, 2);
                double val      = luaL_checknumber(L, 3);
                if (strcmp(key, "x") == 0) {
                    vec->x = static_cast<float>(val);
                } else if (strcmp(key, "y") == 0) {
                    vec->y = static_cast<float>(val);
                }
                return 0;
            },
            1);
        lua_setfield(L, -2, "__newindex");
        lua_setmetatable(L, -2);
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        lua_newtable(L);
        lua_pushnumber(L, value.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, value.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, value.z);
        lua_setfield(L, -2, "z");
        lua_pushnumber(L, value.w);
        lua_setfield(L, -2, "w");

        // Also add common color aliases
        lua_pushnumber(L, value.x);
        lua_setfield(L, -2, "r");
        lua_pushnumber(L, value.y);
        lua_setfield(L, -2, "g");
        lua_pushnumber(L, value.z);
        lua_setfield(L, -2, "b");
        lua_pushnumber(L, value.w);
        lua_setfield(L, -2, "a");

        // Add metatable to handle setting from Lua
        lua_newtable(L); // metatable
        lua_pushlightuserdata(L, (void*) &value);
        lua_pushcclosure(
            L,
            [](lua_State* L) -> int {
                glm::vec4* vec  = (glm::vec4*) lua_touserdata(L, lua_upvalueindex(1));
                const char* key = luaL_checkstring(L, 2);
                double val      = luaL_checknumber(L, 3);
                if (strcmp(key, "x") == 0 || strcmp(key, "r") == 0) {
                    vec->x = static_cast<float>(val);
                } else if (strcmp(key, "y") == 0 || strcmp(key, "g") == 0) {
                    vec->y = static_cast<float>(val);
                } else if (strcmp(key, "z") == 0 || strcmp(key, "b") == 0) {
                    vec->z = static_cast<float>(val);
                } else if (strcmp(key, "w") == 0 || strcmp(key, "a") == 0) {
                    vec->w = static_cast<float>(val);
                }
                return 0;
            },
            1);
        lua_setfield(L, -2, "__newindex");
        lua_setmetatable(L, -2);
    } else if constexpr (std::is_same_v<T, std::vector<glm::vec2>>) {
        lua_newtable(L);
        for (size_t i = 0; i < value.size(); ++i) {
            lua_newtable(L);
            lua_pushnumber(L, value[i].x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, value[i].y);
            lua_setfield(L, -2, "y");
            lua_seti(L, -2, i + 1); // Lua arrays are 1-indexed
        }
    } else if constexpr (std::is_same_v<T, std::string>) {
        lua_pushstring(L, value.c_str());
    } else {
        // Default fallback for unknown types
        lua_pushnil(L);
    }
}

template <typename T>
inline void lua_get_value(lua_State* L, int index, T& value) {
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
        value = static_cast<T>(luaL_checknumber(L, index));
    } else if constexpr (std::is_same_v<T, int>) {
        value = static_cast<T>(luaL_checkinteger(L, index));
    } else if constexpr (std::is_same_v<T, bool>) {
        value = lua_toboolean(L, index) != 0;
    } else if constexpr (std::is_enum_v<T>) {
        value = static_cast<T>(luaL_checkinteger(L, index));
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
        if (lua_istable(L, index)) {
            lua_getfield(L, index, "x");
            if (lua_isnumber(L, -1)) {
                value.x = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "y");
            if (lua_isnumber(L, -1)) {
                value.y = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);
        }
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        if (lua_istable(L, index)) {
            lua_getfield(L, index, "x");
            if (lua_isnumber(L, -1)) {
                value.x = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "y");
            if (lua_isnumber(L, -1)) {
                value.y = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "z");
            if (lua_isnumber(L, -1)) {
                value.z = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "w");
            if (lua_isnumber(L, -1)) {
                value.w = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            // Also check for color aliases (r, g, b, a)
            lua_getfield(L, index, "r");
            if (lua_isnumber(L, -1)) {
                value.x = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "g");
            if (lua_isnumber(L, -1)) {
                value.y = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "b");
            if (lua_isnumber(L, -1)) {
                value.z = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "a");
            if (lua_isnumber(L, -1)) {
                value.w = static_cast<float>(lua_tonumber(L, -1));
            }
            lua_pop(L, 1);
        }
    } else if constexpr (std::is_same_v<T, std::vector<glm::vec2>>) {
        if (lua_istable(L, index)) {
            value.clear();
            lua_pushnil(L); // First key
            while (lua_next(L, index) != 0) {
                if (lua_isnumber(L, -2) && lua_istable(L, -1)) { // Check if key is number and value is table
                    glm::vec2 vertex;
                    lua_getfield(L, -1, "x");
                    if (lua_isnumber(L, -1)) {
                        vertex.x = static_cast<float>(lua_tonumber(L, -1));
                    }
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "y");
                    if (lua_isnumber(L, -1)) {
                        vertex.y = static_cast<float>(lua_tonumber(L, -1));
                    }
                    lua_pop(L, 1);

                    value.push_back(vertex);
                }
                lua_pop(L, 1); // Remove value, keep key for next iteration
            }
        }
    }else if constexpr (std::is_same_v<T, std::string>) {
        if (lua_isstring(L, index)) {
            value = lua_tostring(L, index);
        }
    } else {
        // Default fallback for unknown types
        // Do nothing, keep existing value
    }
}

// ==============================================================
// GENERIC LUA GETTER/SETTER FOR COMPONENT FIELDS               |
// ==============================================================

template <typename T>
inline int lua_get_field(lua_State* L, T& comp) {
    const char* key = luaL_checkstring(L, 2);
    bool found      = false;

    std::apply(
        [&](auto&&... pair) {
            ((found || (std::string(pair.first) == key ? (lua_push_value(L, comp.*(pair.second)), found = true) : false)) || ...);
        },

        Reflected<T>::fields());

    if (!found) {
        lua_pushnil(L);
    }
    return 1;
}

template <typename T>
inline int lua_set_field(lua_State* L, T& comp) {
    const char* key = luaL_checkstring(L, 2);
    // Value is at index 3
    bool found = false;

    std::apply(
        [&](auto&&... pair) {
            ((found || (std::string(pair.first) == key ? (lua_get_value(L, 3, comp.*(pair.second)), found = true) : false)) || ...);
        },
        Reflected<T>::fields());

    if (!found) {
        LOG_WARN("Attempted to set unknown field '%s' on component", key);
    }

    return 0;
}

