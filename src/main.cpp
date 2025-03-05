#include "opengl/glad.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <fstream>
#include <sstream>

struct AppContext {
    SDL_Window* window;
    SDL_GLContext glContext;
    GLuint VAO, VBO, EBO, shaderProgram;
};


std::string LoadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open shader file: %s", filepath.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "shader compilation error %s", infoLog);
    }
    return shader;
}

void InitOpenGL(AppContext* app) {

    auto vertexShaderSource = LoadShaderSource("assets/default.glsl.vert");
    auto fragmentShaderSource = LoadShaderSource("assets/default.glsl.frag");

    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());

    std::cout << "Vertex Shader: " << vertexShader << std::endl;
    std::cout << "Fragment Shader: " << fragmentShader << std::endl;

    app->shaderProgram = glCreateProgram();
    glAttachShader(app->shaderProgram, vertexShader);
    glAttachShader(app->shaderProgram, fragmentShader);
    glLinkProgram(app->shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

    glGenVertexArrays(1, &app->VAO);
    glGenBuffers(1, &app->VBO);

    glBindVertexArray(app->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, app->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("SDL3 opengl", 1280, 720, SDL_WINDOW_OPENGL);

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);


#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 4.1 -> GLSL: 410*/
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#endif

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress);


#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

#endif

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Version %s", glGetString(GL_VERSION));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Vendor %s", glGetString(GL_VENDOR));


    AppContext* app = new AppContext{window, glContext};
    InitOpenGL(app);

    glViewport(0,0,640,360);
    
    *app_state = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void* app_state) {
    auto app = (AppContext*) app_state;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(app->shaderProgram);
    glBindVertexArray(app->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(app->window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {
    auto app = (AppContext*) app_state;

    glDeleteVertexArrays(1, &app->VAO);
    glDeleteBuffers(1, &app->VBO);
    glDeleteProgram(app->shaderProgram);

    SDL_GL_DestroyContext(app->glContext);
    SDL_DestroyWindow(app->window);
    delete app;
}
