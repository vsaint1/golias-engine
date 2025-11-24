#include "servers/rendering/shader_preprocessor.h"
#include <stdafx.h>

ShaderSource parse_shader(const std::string& source) {
    ShaderSource result;

    if (source.find("void main()") != std::string::npos || source.find("#[compute]") != std::string::npos) {
        result.is_compute     = true;
        result.compute_source = source;
        result.is_loaded        = true;
        return result;
    }

    const size_t vertex_pos   = source.find("void vertex");
    const size_t fragment_pos = source.find("void fragment");

    if (vertex_pos == std::string::npos || fragment_pos == std::string::npos) {
        spdlog::error("Shader must have both vertex() and fragment()");
        result.is_loaded = false;
        return result;
    }

    size_t first_main       = std::min(vertex_pos, fragment_pos);
    std::string common_code = source.substr(0, first_main);

    size_t vertex_end       = fragment_pos;
    std::string vertex_code = source.substr(vertex_pos, vertex_end - vertex_pos);

    std::string fragment_code = source.substr(fragment_pos);

    std::string vertex_common   = common_code;
    std::string fragment_common = common_code;

    size_t pos = 0;
    while ((pos = fragment_common.find("out vec", pos)) != std::string::npos) {
        size_t name_start = fragment_common.find_first_not_of(" \t", pos + 7); // after "out vec"
        if (name_start != std::string::npos) {
            size_t var_start = fragment_common.find(' ', name_start);
            if (var_start != std::string::npos) {
                var_start = fragment_common.find_first_not_of(" \t", var_start);
                if (var_start != std::string::npos && fragment_common[var_start] == 'v') {
                    fragment_common.replace(pos, 3, "in ");
                }
            }
        }
        pos += 3;
    }


    pos = 0;
    while ((pos = vertex_common.find("out vec4 COLOR", pos)) != std::string::npos) {
        size_t line_start = vertex_common.rfind('\n', pos);
        if (line_start == std::string::npos) {
            line_start = 0;
        } else {
            line_start++;
        }

        size_t line_end = vertex_common.find(';', pos);
        if (line_end != std::string::npos) {
            line_end = vertex_common.find('\n', line_end);
            if (line_end != std::string::npos) {
                vertex_common.erase(line_start, line_end - line_start + 1);
                pos = line_start;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    result.vertex_source = vertex_common + "\n" + vertex_code;
    size_t vm_pos        = result.vertex_source.find("void vertex()");
    if (vm_pos != std::string::npos) {
        result.vertex_source.replace(vm_pos, 13, "void main()       ");
    }

    result.fragment_source = fragment_common + "\n" + fragment_code;
    size_t fm_pos          = result.fragment_source.find("void fragment()");
    if (fm_pos != std::string::npos) {
        result.fragment_source.replace(fm_pos, 15, "void main()         ");
    }

    spdlog::info("Shader Metadata: Vertex size = {} | Fragment size = {} | Compute: {}", result.vertex_source.size(), result.fragment_source.size(),result.is_compute);
    result.is_loaded = true;
    return result;
}


ShaderSource load_shader_file(const char* filepath) {
    ShaderSource result;

    SDL_IOStream* file = SDL_IOFromFile(filepath,"rb");
    if (!file) {
        spdlog::error("Failed to open Shader source file: {}", filepath);
        result.is_loaded = false;
        return result;
    }

    SDL_SeekIO(file,0,SDL_IO_SEEK_END);
    Sint64 size = SDL_TellIO(file);
    SDL_SeekIO(file,0,SDL_IO_SEEK_SET);

    std::string source;
    source.resize(size);
    SDL_ReadIO(file, &source[0], size);
    SDL_CloseIO(file);

    return parse_shader(source);
}
