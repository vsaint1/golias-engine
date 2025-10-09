#include "core/engine.h"
#include <doctest/doctest.h>

TEST_CASE("Parse config file") {
    EngineConfig config;

    MESSAGE("Load config defaults from project.xml");

    config.load();


    // Application info
    CHECK_EQ(std::string_view(config.get_application().name), "Ember Engine - Window");
    CHECK_EQ(std::string_view(config.get_application().package_name), "com.ember.engine.app");

    // Window info
    CHECK_EQ(config.get_window().width, 1280);
    CHECK_EQ(config.get_window().height, 720);
}
