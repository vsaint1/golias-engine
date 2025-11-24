#pragma once

#include <string>


// TODO: we must create a robust shader preprocessor with naming conventions and more features
struct ShaderSource {
    std::string vertex_source;
    std::string fragment_source;
    std::string compute_source;
    bool is_compute = false;
    bool is_loaded    = false;
};

ShaderSource parse_shader(const std::string& source);

ShaderSource load_shader_file(const char* filepath);
