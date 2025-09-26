#include "core/binding/lua.h"

#include "core/engine.h"

void generate_bindings(sol::state_view lua) {
    sol::usertype<flecs::entity> entity_type = lua.new_usertype<flecs::entity>("EntityHandle", sol::no_constructor);


    lua.new_usertype<Scene>("Scene", sol::no_constructor);


    sol::table scene            = lua.create_table("Scene");
    scene["change_scene"]       = &change_scene;
    scene["get_name"]           = [](flecs::entity e) { return std::string(e.name()); };
    scene["get_entities_count"] = []() -> int { return 20; };


    lua.new_usertype<std::string>("String", sol::constructors<std::string(), std::string(const char*)>(), "c_str", &std::string::c_str,
                                  "size", &std::string::size);

    lua.new_usertype<RendererDevice>("RendererDevice", "backend", &RendererDevice::backend, "texture_filtering",
                                     &RendererDevice::texture_filtering, "get_backend_str", &RendererDevice::get_backend_str);

    lua.new_usertype<EngineConfig>("EngineConfig", "get_application", &EngineConfig::get_application, "get_renderer_device",
                                   &EngineConfig::get_renderer_device, "get_orientation_str", &EngineConfig::get_orientation_str,
                                   "is_vsync", &EngineConfig::is_vsync, "set_vsync", &EngineConfig::set_vsync);

    lua.new_usertype<Transform2D>("Transform2D", "position", &Transform2D::position, "scale", &Transform2D::scale, "rotation",
                                  &Transform2D::rotation);

    lua.new_usertype<Shape>("Shape", "color", &Shape::color, "filled", &Shape::filled);

    lua.new_usertype<Label2D>("Label2D", "text", &Label2D::text, "offset", &Label2D::offset, "color", &Label2D::color, "font_name",
                              &Label2D::font_name, "font_size", &Label2D::font_size);


    lua.new_usertype<glm::vec2>("vec2", sol::constructors<glm::vec2(), glm::vec2(float, float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y);

    lua.new_usertype<glm::vec4>("vec4", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(), "x", &glm::vec4::x, "y",
                                &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w, "r", &glm::vec4::r, "g", &glm::vec4::g, "b",
                                &glm::vec4::b, "a", &glm::vec4::a);

    lua.new_usertype<SDL_Event>("SDL_Event", "type", &SDL_Event::type);
}


void push_entity_to_lua(sol::state_view lua, flecs::entity e) {

    sol::table self = lua.create_table();

    // -------------------------
    // entity metadata
    // -------------------------
    self["id"]       = e.id();
    self["name"]     = e.name();
    self["is_valid"] = e.is_valid();

    // TODO: implement these functions properly
    self["has_component"] = [e](sol::this_state ts, const std::string& component_name) -> bool { return true; };
    self["add_component"] = [e](sol::this_state ts, const std::string& component_name) -> bool { return true; };

    self["get_component"] = [e](sol::this_state ts, const std::string& component_name) -> sol::object {
        return sol::make_object(ts, nullptr);
    };

    self["remove_component"] = [e](sol::this_state ts, const std::string& component_name) -> bool { return true; };

    // -------------------------
    // components (direct members)
    // -------------------------
    if (e.has<Transform2D>()) {
        self["transform"] = std::ref(e.get_mut<Transform2D>());
    }

    if (e.has<Shape>()) {
        self["shape"] = std::ref(e.get_mut<Shape>());
    }

    if (e.has<Label2D>()) {
        self["label"] = std::ref(e.get_mut<Label2D>());
    }

    // -------------------------
    // expose the entity to Lua as `self`
    // scripts can just use `self` directly
    // -------------------------
    lua["self"] = self;
}
