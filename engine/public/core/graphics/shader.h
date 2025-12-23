#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

enum class EShaderStage{
    VERTEX   = 0x1,
    FRAGMENT = 0x2,
    COMPUTE  = 0x4
};

namespace golias {

    class Shader {
    public:
        
        Shader() = default;

        virtual void Bind()   = 0;
        virtual void Unbind() = 0;

        virtual int32_t GetUniformLocation(const std::string_view pName) = 0;

        virtual void SetUniform(const std::string_view pName, UniformValue& value) = 0;

        int32_t GetNativeHandle() const;

        virtual ~Shader() = default;

    protected:
        int32_t handle = -1;

        std::unordered_map<std::string, uint32_t> uniform_location_cache;
    };
}; // namespace golias
