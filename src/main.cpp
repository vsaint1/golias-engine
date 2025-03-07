#include "core/renderer/vertex_buffer.h"
#include "core/renderer/element_buffer.h"
#include <SDL3/SDL_main.h>


struct Renderer {
    unsigned int VAO, shaderProgram;
    VertexBuffer vertexBuffer;
    ElementBuffer elementBuffer;
};

struct AppContext {
    SDL_Window* window;
    SDL_GLContext glContext;
    Renderer renderer;
    SDL_Thread* entityThread;
};

int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

unsigned int mode = GL_FILL;

int EntityHandler(void* data) {
    int ent;

    for (ent = 0; ent < 1'000; ++ent) {
        LOG_INFO("Handling entities %d",ent);

        SDL_Delay(50);
    }

    return ent;
}

/* opengl shader 410 core, opengles shader 300 es */
std::string LoadShaderSource(const std::string& file_path) {


    const auto path = ASSETS_PATH + file_path;

    SDL_IOStream* file_stream = SDL_IOFromFile(path.c_str(), "rb");

    if (!file_stream) {
        LOG_ERROR("Failed to open shader file: %s", SDL_GetError())
        return "";
    }

    auto file_size = SDL_GetIOSize(file_stream);

    if (file_size <= 0) {
        LOG_ERROR("Failed to get file_size: %s", SDL_GetError())
        return "";
    }

    /* BRIEF: for now is more than enough, TODO: use std::vector<char> */
    char buffer[2048];

    if (SDL_ReadIO(file_stream, buffer, sizeof(buffer)) != file_size) {
        LOG_ERROR("Failed to read file: %s", SDL_GetError());
        return "";
    }

    SDL_CloseIO(file_stream);

    return std::string(buffer, file_size);
}

float vertices[] = {
    // 1
    0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f,
    // 2
    0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f


};

unsigned int indices[] = {0, 1, 3, 1, 2, 3};

unsigned int CompileShader(GLenum type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int bStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &bStatus);

    if (!bStatus) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);

        const char* type_str = type == GL_VERTEX_SHADER ? "VERTEX_SHADER" : "FRAGMENT_SHADER";
        LOG_ERROR("Failed to compile shader %s", type_str);
        LOG_INFO("Error: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    SDL_assert(shader != 0);

    return shader;
}

void InitOpenGL(AppContext* app) {

    std::string vertexShaderSource   = LoadShaderSource("shaders/default.glsl.vert");
    std::string fragmentShaderSource = LoadShaderSource("shaders/default.glsl.frag");

    unsigned int vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());


    app->renderer.shaderProgram = glCreateProgram();
    glAttachShader(app->renderer.shaderProgram, vertexShader);
    GL_ERROR();

    glAttachShader(app->renderer.shaderProgram, fragmentShader);
    GL_ERROR();

    glLinkProgram(app->renderer.shaderProgram);
    GL_ERROR();

    glDeleteShader(vertexShader);
    GL_ERROR();

    glDeleteShader(fragmentShader);
    GL_ERROR();

    glGenVertexArrays(1, &app->renderer.VAO);
    GL_ERROR();

    glBindVertexArray(app->renderer.VAO);
    GL_ERROR();


    VertexBuffer vBuffer(vertices, 18 * sizeof(float));

    // VAO
    glEnableVertexAttribArray(0);
    GL_ERROR();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    GL_ERROR();

    ElementBuffer eBuffer(indices, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_ERROR();

    glBindVertexArray(0);
    GL_ERROR();


    glUseProgram(app->renderer.shaderProgram);
    GL_ERROR();

    unsigned int u_Color = glGetUniformLocation(app->renderer.shaderProgram, "u_Color");

    SDL_assert(u_Color != -1);

    glUniform4f(u_Color, 1.0f, 0.0f, 0.0f, 1.0f);
}

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {
    
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
    {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("Window Sample", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);


#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 4.1 -> GLSL: 410*/
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


#endif


    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        LOG_ERROR("Failed to create OpenGL/ES context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

#endif

    LOG_INFO("Version %s", glGetString(GL_VERSION));
    LOG_INFO("Vendor %s", glGetString(GL_VENDOR));


    AppContext* app = new AppContext{window, glContext};
    InitOpenGL(app);

    SDL_Thread* entities = SDL_CreateThread(EntityHandler, "entities", (void*)NULL);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    *app_state = app;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    auto pKey = SDL_GetKeyboardState(0);

    if (pKey[SDL_SCANCODE_E]) {
        mode = GL_LINE;
    }

    if (pKey[SDL_SCANCODE_Q]) {
        mode = GL_FILL;
    }

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void* app_state) {

    auto app = (AppContext*) app_state;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(app->renderer.VAO);
    GL_ERROR();

    glPolygonMode(GL_FRONT_AND_BACK, mode);
    GL_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    GL_ERROR();

    SDL_GL_SwapWindow(app->window);

    SDL_Delay(16); // 60

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {

    auto app = (AppContext*) app_state;

    glDeleteVertexArrays(1, &app->renderer.VAO);
    glDeleteProgram(app->renderer.shaderProgram);

    SDL_GL_DestroyContext(app->glContext);
    SDL_DestroyWindow(app->window);
    delete app;
}
