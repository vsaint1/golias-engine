#include "core/engine_config.h"

#include "core/io/file_system.h"


bool Viewport::load(const tinyxml2::XMLElement* root) {

    if (!root) {
        LOG_ERROR("Failed to load Viewport Config  - root element is null");
        return false;
    }

    const auto viewport_element = root->FirstChildElement("viewport");

    if (!viewport_element) {
        LOG_ERROR("Failed to load Viewport Config - viewport element is null");
        return false;
    }

    if (const auto size = viewport_element->FirstChildElement("size")) {
        size->QueryIntAttribute("width", &width);
        size->QueryIntAttribute("height", &height);
    } else {
        LOG_ERROR("Failed to load Viewport Config - size element is null");
        return false;
    }

    if (const auto stretch = viewport_element->FirstChildElement("stretch")) {


        if (const char* mode_xml = stretch->Attribute("mode")) {
            if (strcmp(mode_xml, "viewport") == 0) {
                mode = ViewportMode::VIEWPORT;
            } else if (strcmp(mode_xml, "canvas") == 0) {
                mode = ViewportMode::CANVAS;
            } else {
                LOG_ERROR("Unknown viewport mode: %s", mode_xml);
                return false;
            }
        }

    } else {
        LOG_ERROR("Failed to load Viewport Config - stretch element is null");
        return false;
    }

    viewport_element->QueryFloatAttribute("scale", &scale);


    return true;
}


bool Environment::load(const tinyxml2::XMLElement* root) {

    if (!root) {
        LOG_ERROR("Failed to load Environment Config - root element is null");
        return false;
    }

    const auto environment_element = root->FirstChildElement("environment");

    if (!environment_element) {
        LOG_ERROR("Failed to load Environment Config - environment element is null");
        return false;
    }

    const auto clear_color_element = environment_element->FirstChildElement("clear_color");

    if (clear_color_element) {
        clear_color.x = clear_color_element->FloatAttribute("r", 0.0f);
        clear_color.y = clear_color_element->FloatAttribute("g", 0.0f);
        clear_color.z = clear_color_element->FloatAttribute("b", 0.0f);
        clear_color.w = clear_color_element->FloatAttribute("a", 1.0f);
    } else {
        LOG_ERROR("Failed to load Environment Config - clear_color element is null");
        return false;
    }

    return true;
}

bool Application::load(const tinyxml2::XMLElement* root) {

    if (!root) {
        LOG_ERROR("Failed to load Application Config - root element is null");
        return false;
    }

    const auto app_element = root->FirstChildElement("application");

    if (!app_element) {
        LOG_ERROR("Failed to load Application Config - application element is null");
        return false;
    }

    if (const auto name_element = app_element->FirstChildElement("name")) {
        name = name_element->GetText();
    } else {
        LOG_WARN("Failed to load Application Config - name element is null");
    }

    if (const auto version_element = app_element->FirstChildElement("version")) {
        version = version_element->GetText();
    } else {
        LOG_WARN("Failed to load Application Config - version element is null");
    }

    if (const auto identifier_element = app_element->FirstChildElement("identifier")) {
        package_name = identifier_element->GetText();
    } else {
        LOG_WARN("Failed to load Application Config - identifier element is null");
    }

    if (const auto icon_element = app_element->FirstChildElement("icon")) {
        icon_path = icon_element->GetText();
    } else {
        LOG_WARN("Failed to load Application Config - icon element is null");
    }

    if (const auto description_element = app_element->FirstChildElement("description")) {
        description = description_element->GetText();
    } else {
        LOG_WARN("Failed to load Application Config - description element is null");
    }

    if (const auto max_fps_element = app_element->FirstChildElement("max_fps")) {
        max_fps_element->QueryIntText(&max_fps);
    } else {
        LOG_WARN("Failed to load Application Config - max_fps element is null");
    }

    return true;
}

std::string EngineConfig::get_orientation_string() const {
    switch (orientation) {
    case Orientation::LANDSCAPE_LEFT: return "LandscapeLeft";
    case Orientation::LANDSCAPE_RIGHT: return "LandscapeRight";
    case Orientation::PORTRAIT: return "Portrait";
    case Orientation::PORTRAIT_UPSIDE_DOWN: return "PortraitUpsideDown";
    default: return "Portrait";
    }
}

bool EngineConfig::load() {


    const auto file = _load_file_into_memory("project.xml");

    if (_doc.Parse(file.data(),file.size()) != tinyxml2::XML_SUCCESS) {
        LOG_ERROR("Failed to load Engine Config from %s, error: %s", "res/project.xml",_doc.ErrorStr());
        return false;
    }

    const tinyxml2::XMLElement* config = _doc.FirstChildElement("config");

    if (!config) {
        LOG_ERROR("Failed to load Engine Config - config element is null");
        return false;
    }

    if (const auto vsync_element = config->FirstChildElement("vsync")) {
        vsync_element->QueryBoolText(&_is_vsync_enabled);
    }

    if (const auto orientation_element = config->FirstChildElement("orientation")) {
        if (const char* orientation_str = orientation_element->Attribute("type")) {
            if (strcmp(orientation_str, "landscape_left") == 0) {
                orientation = Orientation::LANDSCAPE_LEFT;
            } else if (strcmp(orientation_str, "landscape_right") == 0) {
                orientation = Orientation::LANDSCAPE_RIGHT;
            } else if (strcmp(orientation_str, "portrait") == 0) {
                orientation = Orientation::PORTRAIT;
            } else if (strcmp(orientation_str, "portrait_upside_down") == 0) {
                orientation = Orientation::PORTRAIT_UPSIDE_DOWN;
            } else {
                LOG_ERROR("Unknown orientation type: %s", orientation_str);
                return false;
            }
        }
    }

    if (!_viewport.load(config)) {
        LOG_ERROR("Failed to load Viewport Config");
        return false;
    }

    if (!_environment.load(config)) {
        LOG_ERROR("Failed to load Environment Config");
        return false;
    }

    if (!_app.load(config)) {
        LOG_ERROR("Failed to load Application Config");
        return false;
    }

    return true;
}
