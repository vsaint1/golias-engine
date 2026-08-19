#pragma once
#include "stdafx.h"

namespace golias {

    class Shader {

    public:

        Shader(GLuint programID);

        void Bind() const;

        void SetUniform(CString name, int value);

        void SetUniform(CString name, float value);

        void SetUniform(CString name, const glm::vec2& value);

        void SetUniform(CString name, const glm::vec3& value);

        void SetUniform(CString name, const glm::vec4& value);

        ~Shader();

    private:
        GLuint GetUniformLocation(CString name);

        Shader()                         = default;
        Shader(const Shader&)            = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&)                 = delete;
        Shader& operator=(Shader&&)      = delete;

    private:
        std::unordered_map<size_t, GLuint> mUniformLocations = {};
        GLuint mProgramID                                    = 0;
    };
} // namespace golias
