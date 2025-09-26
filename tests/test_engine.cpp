#include "core/engine.h"
#include <doctest/doctest.h>

TEST_CASE("Engine Initialization") {
    CHECK(GEngine->initialize(800, 600, "Test Window") == true);
    CHECK(GEngine->get_window() != nullptr);
    CHECK(GEngine->get_renderer() != nullptr);

}
