#include <doctest/doctest.h>

TEST_CASE("Hello World") {
    CHECK(1 + 1 == 2);
    CHECK(std::string("Hello") + " World" == "Hello World");
}
