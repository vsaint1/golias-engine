#include "servers/rendering/shader_preprocessor.h"


static  void remove_builtin_declarations(std::string& shader_code) {

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
};


static std::string inject_vertex_attributes(const std::string& shader_source) {
    std::stringstream ss;

    ss << "// Vertex Attributes (auto-injected)\n";
    ss << "layout(location = 0) in vec3 VERTEX;\n";
    ss << "layout(location = 1) in vec4 COLOR;\n";
    ss << "layout(location = 2) in vec2 UV;\n\n";

    ss << "// Built-in Varyings (auto-injected)\n";
    ss << "out vec4 v_COLOR;\n";
    ss << "out vec2 v_UV;\n\n";

    ss << shader_source;

    return ss.str();
}

static std::string inject_fragment_inputs(const std::string& shader_source) {
    std::stringstream ss;

    ss << "// Built-in Varyings (auto-injected)\n";
    ss << "in vec4 v_COLOR;\n";
    ss << "in vec2 v_UV;\n\n";

    ss << "// Shader variables (Godot-style)\n";
    ss << "vec3 ALBEDO = vec3(1.0);\n";
    ss << "float ALPHA = 1.0;\n\n";

    ss << "// Aliases for reading\n";
    ss << "#define UV v_UV\n";
    ss << "#define COLOR v_COLOR\n\n";

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
//   - void vertex() {...}   -> becomes vertex shader main()
//   - void fragment() {...} -> becomes fragment shader main()
//   - #[compute] tag for compute shaders
// ============================================================================
ShaderSource parse_shader(const std::string& source) {
    ShaderSource result;

    // ========================================
    // 1. Detect shader type (Compute vs Vertex/Fragment)
    // ========================================
    const bool has_compute_tag = source.find("#[compute]") != std::string::npos;
    const bool has_main        = source.find("void main()") != std::string::npos;
    const bool has_vertex      = source.find("void vertex") != std::string::npos;
    const bool has_fragment    = source.find("void fragment") != std::string::npos;

    // Handle compute shaders (no processing needed)
    if (has_compute_tag || (has_main && !has_vertex && !has_fragment)) {
        result.is_compute     = true;
        result.compute_source = source;
        result.is_loaded      = true;
        spdlog::info("Shader Type: Compute | Size: {} bytes", result.compute_source.size());
        return result;
    }

    // ========================================
    // 2. Add default GLSL version if missing
    // ========================================
    if (source.find("#version") == std::string::npos) {
        result.vertex_source.append("#version 300 es\nprecision highp float;\n\n");
        result.fragment_source.append("#version 300 es\nprecision highp float;\n\n");
        spdlog::debug("Auto-injected: GLSL ES 3.0 version header");
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
    // First, remove any manual built-in declarations from common code
    // Clean common code first
    std::string clean_common_code = common_code;
    remove_builtin_declarations(clean_common_code);

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

        size_t main_end = result.vertex_source.find_last_of('}');
        if (main_end != std::string::npos) {
            std::string varying_passthrough =
                "    // Auto-pass built-in varyings\n"
                "    v_COLOR = COLOR;\n"
                "    v_UV = UV;\n";
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

        // Find the closing brace of main() and inject ALBEDO/ALPHA assignment to fragColor
        size_t main_end = result.fragment_source.find_last_of('}');
        if (main_end != std::string::npos) {
            std::string output_assignment =
                "    // Auto-assign ALBEDO and ALPHA to output\n"
                "    fragColor = vec4(ALBEDO, ALPHA);\n";
            result.fragment_source.insert(main_end, output_assignment);
        }
    }

    // ========================================
    // 9. Finalize and log metadata
    // ========================================

    spdlog::debug("=== Final Vertex Shader ===\n{}", result.vertex_source);
    spdlog::debug("=== Final Fragment Shader ===\n{}", result.fragment_source);

    spdlog::info("Shader Type: Vertex/Fragment | Vertex: {} bytes | Fragment: {} bytes",
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

    // Get  size
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
