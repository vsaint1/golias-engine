#include "core/component/components.h"


Script::~Script() {
    if (lua_state) {

        lua_getglobal(lua_state, "destroy");
        if (lua_isfunction(lua_state, -1)) {
            if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK) {
                const char* err = lua_tostring(lua_state, -1);
                LOG_ERROR("Error running destroy in script %s: %s", path.c_str(), err);
                lua_pop(lua_state, 1);
            }
        }
        lua_close(lua_state);
        lua_state = nullptr;
    }
}
