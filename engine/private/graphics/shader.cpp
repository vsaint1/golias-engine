#include "graphics/shader.h"

#include "core/engine.h"
#include "graphics/shader_compiler.h"
#include "graphics/texture.h"

namespace golias {

    Ref<Shader> Shader::Load(CString path) {
        const String source = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

#if defined(GOLIAS_PLATFORM_WINDOWS) || defined(GOLIAS_PLATFORM_LINUX) || defined(GOLIAS_PLATFORM_OSX)
        constexpr ShaderTarget target = ShaderTarget::OpenGLCore330;
#else
        constexpr ShaderTarget target = ShaderTarget::OpenGLES300;
#endif
        CompiledShaderSource compiled = ShaderCompiler_Compile(source, target, path);

        if (!compiled.IsValid()) {
            GOLIAS_LOG_ERROR("Failed to compile shader '%s'.", path.data());
            return nullptr;
        }

        return Engine::GetInstance().GetGraphicsDevice().CreateShader(compiled.Vertex, compiled.Fragment);
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

    void Shader::SetUniform(CString name, const Texture* texture) {

        GLuint location = GetUniformLocation(name);

        glActiveTexture(GL_TEXTURE0 + mUnitIndex);
        glBindTexture(texture->GetTarget(), texture->GetHandle());
        glUniform1i(location, mUnitIndex);
        mUnitIndex++;
    }

    void Shader::SetTexture(const TextureBinding& binding, const Texture* texture) {
        const GLuint location = GetUniformLocation(binding.Sampler);

        glActiveTexture(GL_TEXTURE0 + binding.Unit);
        glBindTexture(texture->GetTarget(), texture->GetHandle());
        glUniform1i(location, static_cast<GLint>(binding.Unit));
    }

    void Shader::SetUniformBlockBinding(CString name, uint32_t bindingIndex) {
        constexpr size_t kMaxBlockNameLen = 128;
        char buffer[kMaxBlockNameLen];

        const size_t len = std::min(name.size(), kMaxBlockNameLen - 1);
        std::memcpy(buffer, name.data(), len);
        buffer[len] = '\0';

        const GLuint blockIndex = glGetUniformBlockIndex(mProgramID, buffer);
        if (blockIndex == GL_INVALID_INDEX) {
            GOLIAS_LOG_WARN("Uniform block '%s' not found in shader program.", buffer);
            return;
        }

        glUniformBlockBinding(mProgramID, blockIndex, bindingIndex);
    }
} // namespace golias
