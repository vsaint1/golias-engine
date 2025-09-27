#include <doctest/doctest.h>
#include "core/binding/lua.h"

TEST_CASE("Engine Lua Binding") {
    Transform2D transform;
    transform.position = {200, 300};
    transform.scale    = {1, 1};
    transform.rotation = 0;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    generate_bindings(L);

    lua_pushlightuserdata(L, &transform);
    lua_setglobal(L, "transform");

    luaL_dostring(L, R"(
        local speed = 50
        function ready()
            transform.position.x = speed
            transform.position.y = speed  + 20
        end
    )");

    lua_getglobal(L, "ready");
    REQUIRE(lua_pcall(L, 0, 0, 0) == LUA_OK);
}
