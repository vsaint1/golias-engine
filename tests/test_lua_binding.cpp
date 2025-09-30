#include <doctest/doctest.h>
#include "core/binding/lua.h"

TEST_CASE("Engine Lua Binding - Call Function Error Handling") {
    Transform2D transform;
    transform.position = {200, 300};
    transform.scale    = {1, 1};
    transform.rotation = 0;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    generate_bindings(L);

    auto* ptr = (Transform2D*)lua_newuserdata(L, sizeof(Transform2D));
    *ptr = transform;
    luaL_setmetatable(L, "Transform2D");
    lua_setglobal(L, "transform");

    luaL_dostring(L, R"(
        local speed = 50
        function ready()
            transform.position.x = transform.position.x - speed
            transform.position.y = transform.position.y - speed
        end
    )");

    lua_getglobal(L, "ready");
    REQUIRE(lua_pcall(L, 0, 0, 0) == LUA_ERRRUN);

    CHECK(ptr->position.x == 200);
    CHECK(ptr->position.y == 300);

    lua_close(L);
}
