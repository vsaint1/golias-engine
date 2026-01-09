#pragma once
#include "core/graphics/shader.h"
#include <glad.h>

namespace golias {

    class OpenglShader : public Shader {
    public:
        OpenglShader(const std::string& vertex, const std::string& fragment);
        ~OpenglShader() override;

        void Bind() override;
        void Unbind() override;

        int32_t GetUniformLocation(const std::string_view pName) override;
        void SetUniform(const std::string_view pName, const UniformValue& value,int count = 0) override;

    private:
        GLuint CompileShader(GLenum type, const std::string& source);

        GLuint CreateProgram(const std::string& vertexSource, const std::string& fragmentSource);

        const char* GetShaderTypeCStr(GLenum type);

    };

}; // namespace golias
