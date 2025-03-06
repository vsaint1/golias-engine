#include "helpers/logging.h"
#include <SDL3/SDL_main.h>

struct AppContext {
    SDL_Window* window;
    SDL_GLContext glContext;
    GLuint VAO, VBO, EBO, shaderProgram;
};

int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

GLenum mode = GL_TRIANGLES;

/* opengl shader 410 core, opengles shader 300 es */
std::string LoadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open shader file: %s", filepath.c_str())
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

float vertices[] = {
    // 1
     0.5f,  0.5f, 0.0f,  
     0.5f, -0.5f, 0.0f, 
    -0.5f,  0.5f, 0.0f, 
    // 2
     0.5f, -0.5f, 0.0f,  
    -0.5f, -0.5f, 0.0f,  
    -0.5f,  0.5f, 0.0f   
}; 

unsigned int indices[] = {
    0,1,3,
    1,2,3
};

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOG_ERROR("Shader compilation error %s",infoLog);
    }
    return shader;
}

void InitOpenGL(AppContext* app) {

    auto vertexShaderSource = LoadShaderSource(ASSETS_PATH + "shaders/default.glsl.vert");
    auto fragmentShaderSource = LoadShaderSource(ASSETS_PATH + "shaders/default.glsl.frag");

    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());


    app->shaderProgram = glCreateProgram();
    glAttachShader(app->shaderProgram, vertexShader);
    glAttachShader(app->shaderProgram, fragmentShader);
    glLinkProgram(app->shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    glGenVertexArrays(1, &app->VAO);
    glGenBuffers(1, &app->VBO);

    glBindVertexArray(app->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, app->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,app->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0); // XYZ
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 12); // RGB
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Window Sample", SCREEN_WIDTH,SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

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
        LOG_ERROR("Failed to create OpenGL/ES context: %s",SDL_GetError());
        return SDL_APP_FAILURE;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

#endif

    LOG_INFO( "Version %s", glGetString(GL_VERSION));
    LOG_INFO( "Vendor %s", glGetString(GL_VENDOR));


    AppContext* app = new AppContext{window, glContext};
    InitOpenGL(app);

    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    *app_state = app;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }


    auto pKey = SDL_GetKeyboardState(0);
    
    if(pKey[SDL_SCANCODE_E]){
        mode = GL_LINE;
    }

    if(pKey[SDL_SCANCODE_Q]){
        mode = GL_FILL;
    }

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void* app_state) {
    auto app = (AppContext*) app_state;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(app->shaderProgram);
    glBindVertexArray(app->VAO);
    glPolygonMode(GL_FRONT_AND_BACK,mode);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(app->window);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {
    auto app = (AppContext*) app_state;

    glDeleteVertexArrays(1, &app->VAO);
    glDeleteBuffers(1, &app->VBO);
    glDeleteBuffers(1,&app->EBO);
    glDeleteProgram(app->shaderProgram);

    SDL_GL_DestroyContext(app->glContext);
    SDL_DestroyWindow(app->window);
    delete app;
}
