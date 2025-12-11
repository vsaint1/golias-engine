#include "servers/rendering/shader_preprocessor.h"

std::string get_shader_header() {
#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)
    return "#version 300 es\nprecision highp float;\n\n";
#else
    return "#version 330 core\n\n";
#endif
}


static void remove_builtin_declarations(std::string& shader_code) {

    // List of built-in varyings to remove (users shouldn't declare these)
    const char* builtins[] = {
        "out vec4 COLOR;",
        "in vec4 COLOR;",
        "out vec2 UV;",
        "in vec2 UV;",
        "out vec3 VERTEX;",
        "in vec3 VERTEX;",
        "varying COLOR;",
        "varying UV;",
        "varying VERTEX;"
    };

    for (const char* builtin : builtins) {
        size_t pos = 0;
        while ((pos = shader_code.find(builtin, pos)) != std::string::npos) {
            // Find line start
            size_t line_start = shader_code.rfind('\n', pos);
            line_start = (line_start == std::string::npos) ? 0 : line_start + 1;

            // Find line end
            size_t line_end = shader_code.find('\n', pos);
            if (line_end == std::string::npos) {
                line_end = shader_code.length();
            } else {
                line_end++; // Include the newline
            }

            shader_code.erase(line_start, line_end - line_start);
            pos = line_start;
        }
    }
}

static void remove_shader_type_directive(std::string& shader_code) {
    // Remove "shader_type canvas_item;" or "shader_type spatial;" lines
    const char* directives[] = {
        "shader_type canvas_item;",
        "shader_type spatial;"
    };

    for (const char* directive : directives) {
        size_t pos = 0;
        while ((pos = shader_code.find(directive, pos)) != std::string::npos) {
            // Find line start
            size_t line_start = shader_code.rfind('\n', pos);
            line_start = (line_start == std::string::npos) ? 0 : line_start + 1;

            // Find line end
            size_t line_end = shader_code.find('\n', pos);
            if (line_end == std::string::npos) {
                line_end = shader_code.length();
            } else {
                line_end++; // Include the newline
            }

            shader_code.erase(line_start, line_end - line_start);
            pos = line_start;
        }
    }
}


static std::string inject_vertex_attributes(const std::string& shader_source) {
    std::stringstream ss;

    ss << "// Vertex Attributes (auto-injected)\n";
    ss << "layout(location = 0) in vec3 a_vertex;\n";
    ss << "layout(location = 1) in vec4 a_color;\n";
    ss << "layout(location = 2) in vec2 a_uv;\n\n";

    ss << "// Shader variables - writable in vertex shader\n";
    ss << "vec3 VERTEX;\n";
    ss << "vec4 COLOR;\n";
    ss << "vec2 UV;\n\n";

    ss << "// Built-in Varyings (auto-injected)\n";
    ss << "out vec4 v_COLOR;\n";
    ss << "out vec2 v_UV;\n";
    ss << "out vec3 v_VERTEX;\n\n";

    ss << shader_source;

    return ss.str();
}

static std::string inject_fragment_inputs(const std::string& shader_source) {
    std::stringstream ss;

    ss << "// Built-in Varyings (auto-injected)\n";
    ss << "in vec4 v_COLOR;\n";
    ss << "in vec2 v_UV;\n";
    ss << "in vec3 v_VERTEX;\n\n";

    ss << "// Shader variables\n";
    ss << "vec4 COLOR = vec4(1.0);\n";
    ss << "float ALPHA = 1.0;\n\n";

    ss << "// Aliases for reading\n";
    ss << "#define UV v_UV\n";
    ss << "#define VERTEX v_VERTEX\n\n";

    ss << shader_source;

    return ss.str();
}

static std::string inject_builtin_uniforms(const std::string& shader_source) {
    std::stringstream ss;

    ss << "// Built-in Uniforms (auto-injected)\n";
    ss << "uniform mat4 MODEL_MATRIX;\n";
    ss << "uniform mat4 VIEW_MATRIX;\n";
    ss << "uniform mat4 PROJECTION_MATRIX;\n";
    ss << "uniform mat4 VIEW_PROJECTION_MATRIX;\n";
    ss << "uniform float TIME;\n";
    ss << "uniform vec3 CAMERA_POSITION;\n";
    ss << "uniform sampler2D TEXTURE;\n\n";
    ss << shader_source;

    return ss.str();
}

static std::string inject_fragment_outputs(const std::string& shader_source) {
    std::stringstream ss;

    ss << "// Fragment Outputs (auto-injected)\n";
    ss << "out vec4 fragColor;\n\n";
    ss << shader_source;

    return ss.str();
}

// ============================================================================
// Shader Parser - Processes custom shader syntax
// ============================================================================
// Syntax:
//   - shader_type canvas_item; -> 2D shader (supported)
//   - shader_type spatial;     -> 3D shader (not supported)
//   - void vertex() {...}      -> becomes vertex shader main()
//   - void fragment() {...}    -> becomes fragment shader main()
//   - #[compute] tag for compute shaders
// ============================================================================
ShaderSource parse_shader(const std::string& source) {
    ShaderSource result;

    std::string shader_header;

    // ========================================
    // 1. Detect shader type
    // ========================================
    const bool has_compute_tag = source.find("#[compute]") != std::string::npos;
    const bool has_main        = source.find("void main()") != std::string::npos;
    const bool has_vertex      = source.find("void vertex") != std::string::npos;
    const bool has_fragment    = source.find("void fragment") != std::string::npos;

    // Check for shader_type directives
    const bool has_spatial = source.find("shader_type spatial") != std::string::npos;
    const bool has_canvas_item = source.find("shader_type canvas_item") != std::string::npos;

    // Spatial shaders are not supported
    if (has_spatial) {
        spdlog::error("shader_type spatial is not supported");
        result.is_loaded = false;
        return result;
    }

    // Handle compute shaders (no processing needed)
    if (has_compute_tag || (has_main && !has_vertex && !has_fragment)) {

        shader_header = "#version 450";

        result.is_compute     = true;
        result.compute_source = shader_header + source;
        result.is_loaded      = true;
        spdlog::info("Shader Type: Compute | Size: {} bytes", result.compute_source.size());
        return result;
    }

    shader_header = get_shader_header();

    // ========================================
    // 2. Add default GLSL version if missing
    // ========================================
    if (source.find("#version") == std::string::npos) {
        result.vertex_source.append(shader_header);
        result.fragment_source.append(shader_header);
    }

    // ========================================
    // 3. Validate shader has required functions
    // ========================================
    const size_t vertex_pos   = source.find("void vertex");
    const size_t fragment_pos = source.find("void fragment");

    if (vertex_pos == std::string::npos || fragment_pos == std::string::npos) {
        spdlog::error("Shader must define both 'void vertex()' and 'void fragment()'");
        result.is_loaded = false;
        return result;
    }

    // ========================================
    // 4. Split shader into sections
    // ========================================
    // Extract common code (everything before first shader function)
    const size_t first_shader_func = std::min(vertex_pos, fragment_pos);
    std::string common_code        = source.substr(0, first_shader_func);

    // Extract vertex shader code
    const size_t vertex_end  = fragment_pos;
    std::string vertex_code  = source.substr(vertex_pos, vertex_end - vertex_pos);

    // Extract fragment shader code
    std::string fragment_code = source.substr(fragment_pos);

    // ========================================
    // 5. Inject built-ins
    // ========================================
    // Clean common code - remove built-in declarations and shader_type directives
    std::string clean_common_code = common_code;
    remove_builtin_declarations(clean_common_code);
    remove_shader_type_directive(clean_common_code);

    // Vertex shader gets: attributes + varyings + uniforms
    std::string vertex_common = inject_builtin_uniforms(clean_common_code);
    vertex_common = inject_vertex_attributes(vertex_common);

    // Fragment shader gets: varyings + uniforms + outputs
    std::string fragment_common = inject_fragment_inputs(clean_common_code);
    fragment_common = inject_builtin_uniforms(fragment_common);


    // Convert "varying " keyword to "out " in vertex shader and "in " in fragment shader
    size_t pos = 0;
    while ((pos = vertex_common.find("varying ", pos)) != std::string::npos) {
        vertex_common.replace(pos, 8, "out ");
        pos += 4;
    }

    pos = 0;
    while ((pos = fragment_common.find("varying ", pos)) != std::string::npos) {
        fragment_common.replace(pos, 8, "in ");
        pos += 3;
    }

    // handle traditional "out " declarations (convert to "in " for fragment)
    pos = 0;
    while ((pos = fragment_common.find("out ", pos)) != std::string::npos) {
        fragment_common.replace(pos, 4, "in ");
        pos += 3;
    }

    // inject fragment outputs (after varying processing to avoid conversion)
    fragment_common = inject_fragment_outputs(fragment_common);

    // ========================================
    // 6. Assemble final vertex shader
    // ========================================
    result.vertex_source.append(vertex_common);
    result.vertex_source.append("\n");
    result.vertex_source.append(vertex_code);

    // Replace "void vertex()" with "void main()"
    size_t vertex_main_pos = result.vertex_source.find("void vertex()");
    if (vertex_main_pos != std::string::npos) {
        result.vertex_source.replace(vertex_main_pos, 13, "void main()");

        // Find the opening brace of main() and inject initialization
        size_t main_start = result.vertex_source.find('{', vertex_main_pos);
        if (main_start != std::string::npos) {
            std::string init_code =
                "\n    // Initialize shader variables from attributes\n"
                "    VERTEX = a_vertex;\n"
                "    COLOR = a_color;\n"
                "    UV = a_uv;\n\n";
            result.vertex_source.insert(main_start + 1, init_code);
        }

        size_t main_end = result.vertex_source.find_last_of('}');
        if (main_end != std::string::npos) {
            std::string varying_passthrough =
                "    // Auto-pass built-in varyings\n"
                "    v_COLOR = COLOR;\n"
                "    v_UV = UV;\n"
                "    v_VERTEX = VERTEX;\n"
                "    \n"
                "    // Auto-inject gl_Position\n"
                "    gl_Position = VIEW_PROJECTION_MATRIX * vec4(VERTEX, 1.0);\n";
            result.vertex_source.insert(main_end, varying_passthrough);
        }
    }

    // ========================================
    // 8. Assemble final fragment shader
    // ========================================
    result.fragment_source.append(fragment_common);
    result.fragment_source.append("\n");
    result.fragment_source.append(fragment_code);

    // Replace "void fragment()" with "void main()"
    size_t fragment_main_pos = result.fragment_source.find("void fragment()");
    if (fragment_main_pos != std::string::npos) {
        result.fragment_source.replace(fragment_main_pos, 15, "void main()");

        // Find the closing brace of main() and inject COLOR assignment to fragColor
        size_t main_end = result.fragment_source.find_last_of('}');
        if (main_end != std::string::npos) {
            std::string output_assignment =
                "    // Auto-assign COLOR and ALPHA to output\n"
                "    fragColor = vec4(COLOR.rgb, ALPHA);\n";
            result.fragment_source.insert(main_end, output_assignment);
        }
    }

    // ========================================
    // 9. Finalize and log metadata
    // ========================================

    spdlog::debug("=== Final Vertex Shader ===\n{}", result.vertex_source);
    spdlog::debug("=== Final Fragment Shader ===\n{}", result.fragment_source);

    spdlog::info("Shader Type: {} | Vertex: {} bytes | Fragment: {} bytes",
                 has_canvas_item ? "canvas_item (2D)" : "Vertex/Fragment",
                 result.vertex_source.size(), result.fragment_source.size());
    result.is_loaded = true;
    return result;
}

ShaderSource load_shader_file(const char* filepath) {
    ShaderSource result;

    SDL_IOStream* file = SDL_IOFromFile(filepath, "rb");
    if (!file) {
        spdlog::error("Failed to open shader file: {}", filepath);
        result.is_loaded = false;
        return result;
    }

    SDL_SeekIO(file, 0, SDL_IO_SEEK_END);
    Sint64 file_size = SDL_TellIO(file);
    SDL_SeekIO(file, 0, SDL_IO_SEEK_SET);


    std::string shader_source;
    shader_source.resize(file_size);
    SDL_ReadIO(file, &shader_source[0], file_size);
    SDL_CloseIO(file);

    spdlog::debug("Loaded shader file: {} ({} bytes)", filepath, file_size);

    return parse_shader(shader_source);
}