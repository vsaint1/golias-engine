#pragma once
#include "stdafx.h"
#include "graphics/texture_slots.h"

namespace golias {

    class Texture;
    
    class Shader {

    public:

        Shader(GLuint programID);

        static Ref<Shader> Load(CString path);

        void Bind();

        void SetUniform(CString name, int value);

        void SetUniform(CString name, float value);

        void SetUniform(CString name, const glm::vec2& value);

        void SetUniform(CString name, const glm::vec3& value);

        void SetUniform(CString name, const glm::vec4& value);

        void SetUniform(CString name, const glm::mat3& value);
        
        void SetUniform(CString name, const glm::mat4& value);

        void SetUniform(CString name, const Texture* texture);

        void SetTexture(const TextureBinding& binding, const Texture* texture);

        ~Shader();

    private:
        GLuint GetUniformLocation(CString name);

        Shader()                         = default;
        Shader(const Shader&)            = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&)                 = delete;
        Shader& operator=(Shader&&)      = delete;

    private:
        uint32_t mUnitIndex = 0;
        std::unordered_map<size_t, GLuint> mUniformLocations = {};
        GLuint mProgramID                                    = 0;
    };
} // namespace golias
