#pragma once
#include "stdafx.h"

namespace golias {
    constexpr size_t kMaxLights = 32;

    struct alignas(16) GpuLight {
        glm::vec4 Position;
        glm::vec4 Direction;
        glm::vec4 ColorIntensity;
        float Range;
        float SpotAngle;
        int Type;
        int IsShadowCaster;
    };

    static_assert(sizeof(GpuLight) == 64, "GpuLight must match the std140 light layout");

    struct alignas(16) GpuLighting {
        int Count;
        int Padding0;
        int Padding1;
        int Padding2;
        GpuLight Lights[kMaxLights];
    };
}
