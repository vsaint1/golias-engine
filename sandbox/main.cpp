#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <glad.h>
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>

std::string vertex_source = R"(
#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;

out vec3 v_color;
out vec2 v_texcoord;

void main() {
    gl_Position = vec4(a_pos, 1.0);
    v_color = a_color;
    v_texcoord = a_texcoord;
}
)";

std::string fragment_source = R"(
#version 330 core
in vec3 v_color;
in vec2 v_texcoord;

out vec4 frag_color;

void main() {
    frag_color = vec4(v_color, 1.0);
}
)";


GLuint CompileShader(GLenum type, const std::string& source) {
    GLuint shader   = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        spdlog::error("Shader compilation failed: {}, Type: {}", infoLog, type);
    }

    return shader;
}


GLuint CreateShaderProgram(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        spdlog::error("Shader program linking failed: {}", infoLog);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}


struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texcoord;
};


int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Hello World", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
        spdlog::error("Failed to create window: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }


#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    #if defined(SDL_PLATFORM_MACOS)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // OSX compatibility Bit
    #endif


#endif

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if (!gl_context) {
        spdlog::critical("Failed to create OpenGL/ES context: {}", SDL_GetError());
        return false;
    }


#if defined(SDL_PLATFORM_ANDROID) && !defined(SDL_PLATFORM_IOS) && !defined(SDL_PLATFORM_EMSCRIPTEN)
    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize OpenGL Loader (GLAD)");
        return false;
    }
#else
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize OpenGLES Loader (GLAD)");
        return false;
    }
#endif

    spdlog::info("SDL Window created successfully!");

    Vertex triangle[3] = {
        {{-0.5f, -0.5f, 0.0f}, {1, 0, 0}, {0, 0}}, {{0.5f, -0.5f, 0.0f}, {0, 1, 0}, {1, 0}}, {{0.0f, 0.5f, 0.0f}, {0, 0, 1}, {0.5, 1}}};

    GLuint vertex_shader   = CompileShader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_source);

    GLuint shader_program = CreateShaderProgram(vertex_shader, fragment_shader);


    GLuint VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), &triangle, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool is_running = true;

    SDL_Event event;
    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                spdlog::info("Quit event received, exiting.");
                is_running = false;
                break;
            }
        }

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
