#pragma once
#include "stdafx.h"

namespace golias {
    constexpr size_t kMaxLights         = 32;
    constexpr size_t kMaxShadowCascades = 4;

    namespace GpuLayout {
        inline constexpr uint32_t LightingBinding    = 0;
        inline constexpr uint32_t FrameBinding       = 1;
        inline constexpr uint32_t JointsBinding      = 2;
        inline constexpr uint32_t ObjectBinding      = 3;
        inline constexpr uint32_t MaterialBinding    = 4;
        inline constexpr uint32_t PostProcessBinding = 5;

        inline constexpr CString LightBlock       = "PerLight";
        inline constexpr CString FrameBlock       = "PerFrame";
        inline constexpr CString ObjectBlock      = "PerObject";
        inline constexpr CString MaterialBlock    = "PerMaterial";
        inline constexpr CString LightingBlock    = "Lighting";
        inline constexpr CString JointsBlock      = "JointMatrices";
        inline constexpr CString PostProcessBlock = "PostProcess";
    } // namespace GpuLayout

    struct alignas(16) GpuFrame {
        glm::mat4 View;
        glm::mat4 Projection;
        glm::mat4 Ortho;
        glm::vec4 CameraPosition;
        glm::vec4 ShadowSplits;
        glm::mat4 ShadowMatrices[kMaxShadowCascades];
        glm::mat4 ShadowViewProjection;
    };

    struct alignas(16) GpuObject {
        glm::mat4 Model;
        glm::ivec4 Flags; // Object flags (x=IsInstanced, y=IsSkinned )
        glm::vec4 SpritePivotSize;
        glm::vec4 SpriteUvBounds;
        glm::vec4 NormalMatrix[3]; // Matrix3x3
    };

    struct alignas(16) GpuMaterial {
        glm::vec4 BaseColor;
    };

    struct alignas(16) GpuPostProcess {

        // Tonemap parameters
        float Exposure;
        int Tonemap;

        // FXAA parameters
        float TexelSizeX;
        float TexelSizeY;
        float SubpixelQuality;
        float EdgeThreshold;
        float EdgeThresholdMin;
        float _Padding0;
    };

    static_assert(sizeof(GpuFrame) % 16 == 0, "GpuFrame must match the std140 frame layout");
    static_assert(sizeof(GpuObject) % 16 == 0, "GpuObject must match the std140 object layout");
    static_assert(sizeof(GpuMaterial) % 16 == 0, "GpuMaterial must match the std140 material layout");
    static_assert(sizeof(GpuPostProcess) % 16 == 0, "GpuPostProcess must be a multiple of 16 bytes");

    struct alignas(16) GpuLight {
        glm::vec4 Position;
        glm::vec4 DirectionInvRange;
        glm::vec4 ColorIntensity;
        float SpotCutoff;
        int Type;
        int IsShadowCaster;
        int Padding;
    };

    static_assert(sizeof(GpuLight) % 16 == 0, "GpuLight must match the std140 light layout");

    struct alignas(16) GpuLighting {
        int Count;
        int Padding0;
        int Padding1;
        int Padding2;
        GpuLight Lights[kMaxLights];
    };
} // namespace golias
