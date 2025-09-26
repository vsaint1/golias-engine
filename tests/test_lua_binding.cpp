#include <doctest/doctest.h>
#include "core/binding/lua.h"

TEST_CASE("Engine Lua Binding") {
    Transform2D transform;
    transform.position = {200, 300};
    transform.scale    = {1, 1};
    transform.rotation = 0;

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math);

    generate_bindings(lua);
    
    lua["transform"] = &transform;

    lua.script(R"(
        local speed = 50
        function ready()
            transform.position.x = speed 
            transform.position.y = speed  + 20
        end
    )");

    auto ready = lua["ready"];

    REQUIRE(ready.valid());

    ready();

    REQUIRE(transform.position.x == 50);
    REQUIRE(transform.position.y == 70);
}
