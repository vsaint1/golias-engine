#include "core/engine.h"
#include <doctest/doctest.h>

TEST_CASE("Parse config file") {
    EngineConfig config;

    MESSAGE("Load config defaults from project.xml");

    CHECK(config.load()== false);

    CHECK_MESSAGE(std::string(config.get_application().name).compare("Window - Ember Engine") == 0, "Application name should be 'Window - Ember Engine'");
    CHECK_MESSAGE(std::string(config.get_application().package_name).compare("com.ember.engine.app") == 0,
                  "Application package name should be 'com.ember.engine.app'");

    CHECK_MESSAGE(config.get_window().width== 1280, "Window width should be 1280");
    CHECK_MESSAGE(config.get_window().height == 720, "Window height should be 720");
}
