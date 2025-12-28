#pragma once
#include "core/graphics/texture_2d.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <variant>

#include <glm/glm.hpp>

using UniformValue = std::variant<bool,int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, std::shared_ptr<golias::Texture2D>>;

enum class EShaderStage { VERTEX = 0x1, FRAGMENT = 0x2, COMPUTE = 0x4 };

namespace golias {

    class Shader {
    public:
        Shader() = default;

        virtual void Bind()   = 0;
        virtual void Unbind() = 0;

        virtual int32_t GetUniformLocation(const std::string_view pName) = 0;

        virtual void SetUniform(const std::string_view pName, const UniformValue& value) = 0;

        int32_t GetNativeHandle() const;

        virtual ~Shader() = default;

    protected:
        int32_t handle = -1;
        std::unordered_map<std::string, uint32_t> uniform_location_cache;

        int current_texture_unit = 0;
    };
}; // namespace golias
