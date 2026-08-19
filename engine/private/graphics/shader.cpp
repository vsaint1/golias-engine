#include "graphics/shader.h"

namespace golias {

    Shader::Shader(GLuint programID) : mProgramID(programID) {
    }

    void Shader::Bind() const {
        glUseProgram(mProgramID);
    }

    Shader::~Shader() {
        if (mProgramID != 0) {
            glDeleteProgram(mProgramID);
        }
    }

    GLuint Shader::GetUniformLocation(CString name) {
        const size_t hash = std::hash<CString>{}(name);

        if (auto it = mUniformLocations.find(hash); it != mUniformLocations.end()) {
            return it->second;
        }

        constexpr size_t kMaxUniformNameLen = 256;
        char buffer[kMaxUniformNameLen];

        const size_t len = std::min(name.size(), kMaxUniformNameLen - 1);
        std::memcpy(buffer, name.data(), len);
        buffer[len] = '\0';

        if (name.size() >= kMaxUniformNameLen) {
            GOLIAS_LOG_WARN(
                "Uniform name '%.*s' truncated (exceeds %zu chars).", static_cast<int>(name.size()), name.data(), kMaxUniformNameLen - 1);
        }

        const GLint location = glGetUniformLocation(mProgramID, buffer);

        if (location == -1) {
            GOLIAS_LOG_WARN("Uniform '%s' not found in shader program.", buffer);
            return 0;
        }

        mUniformLocations[hash] = static_cast<GLuint>(location);
        return static_cast<GLuint>(location);
    }


    void Shader::SetUniform(CString name, int value) {
        GLuint location = GetUniformLocation(name);
        glUniform1i(location, value);
    }

    void Shader::SetUniform(CString name, float value) {
        GLuint location = GetUniformLocation(name);
        glUniform1f(location, value);
    }

    void Shader::SetUniform(CString name, const glm::vec2& value) {
        GLuint location = GetUniformLocation(name);
        glUniform2fv(location, 1, &value[0]);
    }

    void Shader::SetUniform(CString name, const glm::vec3& value) {
        GLuint location = GetUniformLocation(name);
        glUniform3fv(location, 1, &value[0]);
    }

    void Shader::SetUniform(CString name, const glm::vec4& value) {
        GLuint location = GetUniformLocation(name);
        glUniform4fv(location, 1, &value[0]);
    }

} // namespace golias
