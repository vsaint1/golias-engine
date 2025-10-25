#include "core/utility/project_config.h"

// -----------------------------------------------------------------------------
// Viewport
// -----------------------------------------------------------------------------
bool Viewport::load(const tinyxml2::XMLElement* root) {

    if (!root) {
        spdlog::error("Failed to load Viewport Config  - root element is null");
        return false;
    }

    const auto viewport_element = root->FirstChildElement("viewport");

    if (!viewport_element) {
        spdlog::error("Failed to load Viewport Config - viewport element is null");
        return false;
    }

    if (const auto size = viewport_element->FirstChildElement("size")) {
        size->QueryIntAttribute("width", &width);
        size->QueryIntAttribute("height", &height);
    } else {
        spdlog::error("Failed to load Viewport Config - size element is null");
        return false;
    }

    if (const auto stretch = viewport_element->FirstChildElement("stretch")) {

        if (const char* mode_xml = stretch->Attribute("mode")) {
            if (strcmp(mode_xml, "viewport") == 0) {
                mode = ViewportMode::VIEWPORT;
            } else if (strcmp(mode_xml, "canvas") == 0) {
                mode = ViewportMode::CANVAS;
            } else {
                spdlog::error("Unknown viewport mode: {}", mode_xml);
                return false;
            }
        }

    } else {
        spdlog::error("Failed to load Viewport Config - stretch element is null");
        return false;
    }

    viewport_element->QueryFloatAttribute("scale", &scale);

    return true;
}

// -----------------------------------------------------------------------------
// Environment
// -----------------------------------------------------------------------------
bool Environment::load(const tinyxml2::XMLElement* root) {

    if (!root) {
        spdlog::error("Failed to load Environment Config - root element is null");
        return false;
    }

    const auto environment_element = root->FirstChildElement("environment");

    if (!environment_element) {
        spdlog::error("Failed to load Environment Config - environment element is null");
        return false;
    }

    const auto clear_color_element = environment_element->FirstChildElement("clear_color");

    if (clear_color_element) {
        clear_color.x = clear_color_element->FloatAttribute("r", 0.0f);
        clear_color.y = clear_color_element->FloatAttribute("g", 0.0f);
        clear_color.z = clear_color_element->FloatAttribute("b", 0.0f);
        clear_color.w = clear_color_element->FloatAttribute("a", 1.0f);
    } else {
        spdlog::error("Failed to load Environment Config - clear_color element is null");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------
bool Application::load(const tinyxml2::XMLElement* root) {

    if (!root) {
        spdlog::error("Failed to load Application Config - root element is null");
        return false;
    }

    const auto app_element = root->FirstChildElement("application");

    if (!app_element) {
        spdlog::error("Failed to load Application Config - application element is null");
        return false;
    }

    if (const auto name_element = app_element->FirstChildElement("name")) {
        name = name_element->GetText();
    } else {
        spdlog::warn("Failed to load Application Config - name element is null");
    }

    if (const auto version_element = app_element->FirstChildElement("version")) {
        version = version_element->GetText();
    } else {
        spdlog::warn("Failed to load Application Config - version element is null");
    }

    if (const auto identifier_element = app_element->FirstChildElement("identifier")) {
        package_name = identifier_element->GetText();
    } else {
        spdlog::warn("Failed to load Application Config - identifier element is null");
    }

    if (const auto icon_element = app_element->FirstChildElement("icon")) {
        icon_path = icon_element->GetText();
    } else {
        spdlog::warn("Failed to load Application Config - icon element is null");
    }

    if (const auto description_element = app_element->FirstChildElement("description")) {
        description = description_element->GetText();
    } else {
        spdlog::warn("Failed to load Application Config - description element is null");
    }

    if (const auto max_fps_element = app_element->FirstChildElement("max_fps")) {
        max_fps_element->QueryIntText(&max_fps);
    } else {
        spdlog::warn("Failed to load Application Config - max_fps element is null");
    }

    if (const auto fullscreen_element = app_element->FirstChildElement("fullscreen")) {
        fullscreen_element->QueryBoolText(&is_fullscreen);
    } else {
        spdlog::warn("Failed to load Application Config - fullscreen element is null");
    }

    if (const auto resizable_element = app_element->FirstChildElement("resizable")) {
        resizable_element->QueryBoolText(&is_resizable);
    } else {
        spdlog::warn("Failed to load Application Config - resizable element is null");
    }

    return true;
}

// -----------------------------------------------------------------------------
// EngineConfig orientation
// -----------------------------------------------------------------------------
const char* EngineConfig::get_orientation_str() const {
    switch (orientation) {
    case Orientation::LANDSCAPE_LEFT:
        return "LandscapeLeft";
    case Orientation::LANDSCAPE_RIGHT:
        return "LandscapeRight";
    case Orientation::PORTRAIT:
        return "Portrait";
    case Orientation::PORTRAIT_UPSIDE_DOWN:
        return "PortraitUpsideDown";
    default:
        return "Portrait";
    }
}

// -----------------------------------------------------------------------------
// Performance
// -----------------------------------------------------------------------------
bool Performance::load(const tinyxml2::XMLElement* root) {
    const auto performance_element = root->FirstChildElement("performance");

    if (const auto multithread_element = performance_element->FirstChildElement("multithreading")) {
        multithread_element->QueryBoolText(&is_multithreaded);
    } else {
        spdlog::error("Failed to load Performance Config - multithreading element is null");
        return false;
    }

    if (const auto worker_threads_element = performance_element->FirstChildElement("worker_threads")) {
        worker_threads_element->QueryIntText(&worker_threads);
    } else {
        spdlog::error("Failed to load Performance Config - worker_threads element is null");
        return false;
    }

    if (const auto physics_fps_element = performance_element->FirstChildElement("physics_fps")) {
        physics_fps_element->QueryIntText(&physics_fps);
    } else {
        spdlog::error("Failed to load Performance Config - physics_fps element is null");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// RendererDevice
// -----------------------------------------------------------------------------
bool RendererDevice::load(const tinyxml2::XMLElement* root) {

    const auto renderer_element = root->FirstChildElement("renderer");

    if (const auto method_element = renderer_element->FirstChildElement("method")) {
        const char* method_str = method_element->GetText();
        if (strcmp(method_str, "gl_compatibility") == 0) {
            backend = Backend::GL_COMPATIBILITY;
        } else if (strcmp(method_str, "metal") == 0) {
            backend = Backend::METAL;
        } else if (strcmp(method_str, "vk_forward") == 0) {
            backend = Backend::VK_FORWARD;
        } else if (strcmp(method_str, "directx12") == 0) {
            backend = Backend::DIRECTX12;
        } else if (strcmp(method_str, "auto") == 0) {
            backend = Backend::AUTO;
        } else {
            spdlog::error("Unknown renderer method: {}", method_str);
            return false;
        }
    } else {
        spdlog::error("Failed to load Renderer Config - method element is null");
        return false;
    }

    if (const auto filtering_element = renderer_element->FirstChildElement("texture_filter")) {
        const char* filtering_str = filtering_element->GetText();
        if (strcmp(filtering_str, "nearest") == 0) {
            texture_filtering = TextureFiltering::NEAREST;
        } else if (strcmp(filtering_str, "linear") == 0) {
            texture_filtering = TextureFiltering::LINEAR;
        } else {
            spdlog::error("Unknown texture filtering method: {}", filtering_str);
            return false;
        }
    } else {
        spdlog::error("Failed to load Renderer Config - texture_filter element is null");
        return false;
    }

    return true;
}

const char* RendererDevice::get_backend_str() const {
    switch (backend) {
    case Backend::GL_COMPATIBILITY:
        return "OpenGL/ES 3.3 Compatibility";
    case Backend::METAL:
        return "Metal";
    case Backend::VK_FORWARD:
        return "Vulkan";
    case Backend::DIRECTX12:
        return "DirectX 12";
    case Backend::AUTO:
        return "Auto (SDL_RENDERER_GPU)";
    default:
        return "Unknown";
    }
}

// -----------------------------------------------------------------------------
// Window
// -----------------------------------------------------------------------------
bool Window::load(const tinyxml2::XMLElement* root) {
    return true;
}

// -----------------------------------------------------------------------------
// EngineConfig
// -----------------------------------------------------------------------------
bool EngineConfig::load() {

    const auto file = load_file_into_memory("project.xml");

    if (_doc.Parse(file.data(), file.size()) != tinyxml2::XML_SUCCESS) {
        spdlog::error("Failed to load Engine Config from {}, error: {}", "res/project.xml", _doc.ErrorStr());
        return false;
    }

    const tinyxml2::XMLElement* config = _doc.FirstChildElement("config");

    if (!config) {
        spdlog::error("Failed to load Engine Config - config element is null");
        return false;
    }

    if (const auto vsync_element = config->FirstChildElement("vsync")) {
        vsync_element->QueryBoolText(&_is_vsync_enabled);
    }

    if (const auto orientation_element = config->FirstChildElement("orientation")) {
        if (const char* orientation_str = orientation_element->GetText()) {
            if (strcmp(orientation_str, "landscape_left") == 0) {
                orientation = Orientation::LANDSCAPE_LEFT;
            } else if (strcmp(orientation_str, "landscape_right") == 0) {
                orientation = Orientation::LANDSCAPE_RIGHT;
            } else if (strcmp(orientation_str, "portrait") == 0) {
                orientation = Orientation::PORTRAIT;
            } else if (strcmp(orientation_str, "portrait_upside_down") == 0) {
                orientation = Orientation::PORTRAIT_UPSIDE_DOWN;
            } else {
                spdlog::error("Unknown orientation type: {}", orientation_str);
                return false;
            }
        }
    }

    if (!_viewport.load(config)) {
        spdlog::error("Failed to load Viewport Config");
        return false;
    }

    if (!_environment.load(config)) {
        spdlog::error("Failed to load Environment Config");
        return false;
    }

    if (!_app.load(config)) {
        spdlog::error("Failed to load Application Config");
        return false;
    }

    if (!_renderer_device.load(config)) {
        spdlog::error("Failed to load Renderer Device");
        return false;
    }

    if (!_performance.load(config)) {
        spdlog::error("Failed to load Performance Config");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// EngineConfig getters/setters
// -----------------------------------------------------------------------------
RendererDevice& EngineConfig::get_renderer_device() {
    return _renderer_device;
}

Performance& EngineConfig::get_performance() {
    return _performance;
}

Application& EngineConfig::get_application() {
    return _app;
}

Environment& EngineConfig::get_environment() {
    return _environment;
}

Viewport& EngineConfig::get_viewport() {
    return _viewport;
}

Window& EngineConfig::get_window() {
    return _window;
}

bool EngineConfig::is_vsync() const {
    return _is_vsync_enabled;
}

void EngineConfig::set_vsync(bool enabled) {
    _is_vsync_enabled = enabled;
}
