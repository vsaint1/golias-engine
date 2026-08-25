#include "graphics/shader.h"

#include "core/engine.h"
#include "graphics/texture.h"

namespace golias {

    namespace {
        String PrepareShaderStage(String source) {
#if defined(GOLIAS_PLATFORM_WINDOWS) || defined(GOLIAS_PLATFORM_LINUX) || defined(GOLIAS_PLATFORM_OSX)
            constexpr CString version = "#version 330 core";
#else
            constexpr CString version = "#version 300 es";
#endif

            const size_t versionStart = source.find("#version");
            if (versionStart == String::npos) {
                GOLIAS_LOG_ERROR("Shader stage is missing a #version directive.");
                return {};
            }

            const size_t versionEnd = source.find('\n', versionStart);
            source.replace(
                versionStart, versionEnd == String::npos ? String::npos : versionEnd - versionStart, version.data(), version.size());

#if !defined(GOLIAS_PLATFORM_WINDOWS) && !defined(GOLIAS_PLATFORM_LINUX) && !defined(GOLIAS_PLATFORM_OSX)
            const size_t precisionEnd = source.find('\n', versionStart);
            source.insert(precisionEnd == String::npos ? source.size() : precisionEnd, "\nprecision highp float;\nprecision highp int;");
#endif

            return source;
        }
    } // namespace

    Ref<Shader> Shader::Load(CString path) {
        const String source         = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
        const size_t vertexMarker   = source.find("@vertex");
        const size_t fragmentMarker = source.find("@fragment");

        if (vertexMarker == String::npos || fragmentMarker == String::npos || vertexMarker >= fragmentMarker) {
            GOLIAS_LOG_ERROR("Shader '%s' must contain @vertex followed by @fragment.", path.data());
            return nullptr;
        }

        const size_t vertexStart   = source.find('\n', vertexMarker);
        const size_t fragmentStart = source.find('\n', fragmentMarker);
        if (vertexStart == String::npos || fragmentStart == String::npos) {
            GOLIAS_LOG_ERROR("Shader '%s' has an invalid stage marker.", path.data());
            return nullptr;
        }

        const String vertex   = PrepareShaderStage(source.substr(vertexStart + 1, fragmentMarker - vertexStart - 1));
        const String fragment = PrepareShaderStage(source.substr(fragmentStart + 1));
        if (vertex.empty() || fragment.empty()) {
            return nullptr;
        }

        return Engine::GetInstance().GetGraphicsDevice().CreateShader(vertex, fragment);
    }

    Shader::Shader(GLuint programID) : mProgramID(programID) {
    }

    void Shader::Bind() {
        glUseProgram(mProgramID);
        mUnitIndex = 0; // Reset
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

    void Shader::SetUniform(CString name, const glm::mat3& value) {
        GLuint location = GetUniformLocation(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetUniform(CString name, const glm::mat4& value) {
        GLuint location = GetUniformLocation(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetUniform(CString name, const Texture2D* texture) {

        GLuint location = GetUniformLocation(name);

        glActiveTexture(GL_TEXTURE0 + mUnitIndex);
        glBindTexture(GL_TEXTURE_2D, texture->GetHandle());
        glUniform1i(location, mUnitIndex);
        mUnitIndex++;
    }
} // namespace golias
