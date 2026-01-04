#include "core/graphics/gles3/gl_physics_debug_drawer.h"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "core/graphics/gles3/gl_common.h"

namespace golias {

    static std::string debugVertexShaderSource = GetShaderHeaderVersion() + R"(
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

uniform mat4 VIEW_PROJECTION;

out vec3 v_color;

void main()
{
    gl_Position = VIEW_PROJECTION * vec4(a_pos, 1.0);
    v_color = a_color;
}
)";

    static std::string debugFragmentShaderSource = GetShaderHeaderVersion() + R"(
in vec3 v_color;
out vec4 COLOR;

void main()
{
    COLOR = vec4(v_color, 1.0);
}
)";

    PhysicsDebugDrawerGLES3::PhysicsDebugDrawerGLES3()
        : debugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints), VAO(0), VBO(0),
          shaderProgram(0) {

    }

    PhysicsDebugDrawerGLES3::~PhysicsDebugDrawerGLES3() {
        Cleanup();
    }

    void PhysicsDebugDrawerGLES3::Initialize() {
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* debugVertexShaderSourceCStr = debugVertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &debugVertexShaderSourceCStr, nullptr);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            spdlog::error("PhysicsDebugDrawerGLES3::Initialize Vertex Shader compilation failed: {}", infoLog);
        }

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* debugFragmentShaderSourceCStr = debugFragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &debugFragmentShaderSourceCStr, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            spdlog::error("PhysicsDebugDrawerGLES3::Initialize Fragment Shader compilation failed: {}", infoLog);
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            spdlog::error("PhysicsDebugDrawerGLES3::Initialize Shader Program linking failed: {}", infoLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        spdlog::info("PhysicsDebugDrawerGLES3 initialized");
    }

    void PhysicsDebugDrawerGLES3::Cleanup() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
        if (shaderProgram != 0) {
            glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }
    }

    void PhysicsDebugDrawerGLES3::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
        DebugLine line;
        line.from  = btToGlm(from);
        line.to    = btToGlm(to);
        line.color = btToGlm(color);
        lines.push_back(line);
    }

    void PhysicsDebugDrawerGLES3::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
                                              int lifeTime, const btVector3& color) {
        const btScalar crossSize = 0.1f;
        btVector3 normal         = normalOnB.normalized();
        btVector3 tangent1, tangent2;

        if (btFabs(normal.x()) < 0.9f) {
            tangent1 = btVector3(1, 0, 0).cross(normal);
        } else {
            tangent1 = btVector3(0, 1, 0).cross(normal);
        }

        tangent1.normalize();
        tangent2 = normal.cross(tangent1);

        drawLine(PointOnB - tangent1 * crossSize, PointOnB + tangent1 * crossSize, color);
        drawLine(PointOnB - tangent2 * crossSize, PointOnB + tangent2 * crossSize, color);

        drawLine(PointOnB, PointOnB + normal * 0.3f, btVector3(1, 1, 0));
    }

    void PhysicsDebugDrawerGLES3::reportErrorWarning(const char* warningString) {
        spdlog::warn("PhysicsDebugDrawerGLES3::reportErrorWarning: {}", warningString);
    }

    void PhysicsDebugDrawerGLES3::draw3dText(const btVector3& location, const char* textString) {
       
    }

    void PhysicsDebugDrawerGLES3::setDebugMode(int mode) {
        debugMode = mode;
    }

    int PhysicsDebugDrawerGLES3::getDebugMode() const {
        return debugMode;
    }

    void PhysicsDebugDrawerGLES3::Begin() {
        lines.clear();
    }

    void PhysicsDebugDrawerGLES3::End() {
    }

    void PhysicsDebugDrawerGLES3::Render(const glm::mat4& viewProjection) {
        if (lines.empty()) {
            return;
        }

        std::vector<float> vertexData;
        vertexData.reserve(lines.size() * 12); 

        for (const auto& line : lines) {
            // First vertex (from)
            vertexData.push_back(line.from.x);
            vertexData.push_back(line.from.y);
            vertexData.push_back(line.from.z);
            vertexData.push_back(line.color.r);
            vertexData.push_back(line.color.g);
            vertexData.push_back(line.color.b);

            // Second vertex (to)
            vertexData.push_back(line.to.x);
            vertexData.push_back(line.to.y);
            vertexData.push_back(line.to.z);
            vertexData.push_back(line.color.r);
            vertexData.push_back(line.color.g);
            vertexData.push_back(line.color.b);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

        glUseProgram(shaderProgram);

        int vpLoc = glGetUniformLocation(shaderProgram, "VIEW_PROJECTION");
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));

        GLboolean depthTestEnabled;
        glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() * 2));
        glBindVertexArray(0);

        if (depthTestEnabled) {
            glEnable(GL_DEPTH_TEST);
        }

        glUseProgram(0);
    }

    void PhysicsDebugDrawerGLES3::Clear() {
        lines.clear();
    }

    glm::vec3 PhysicsDebugDrawerGLES3::btToGlm(const btVector3& vec) const {
        return glm::vec3(vec.x(), vec.y(), vec.z());
    }

} // namespace golias
