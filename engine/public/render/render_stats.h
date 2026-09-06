#pragma once
#include "stdafx.h"

namespace golias {

    struct RenderStats {
        float FrameTimeMs   = 0.0f;
        float CpuTimeMs     = 0.0f;
        float GpuTimeMs     = 0.0f;
        uint32_t Fps        = 0;
        bool GpuTiming      = false;
        uint32_t FrameIndex = 0;
        uint32_t DrawCalls  = 0;
        uint32_t Batches    = 0;
        uint32_t Vertices   = 0;
        uint32_t Triangles  = 0;
    };

    /// @brief  Per-frame render performance collector.
    class FrameStats {
    public:
        static void BeginFrame();

        static void RecordDrawCall(uint32_t vertices, uint32_t indices);

        static void RecordCanvasBatch(uint32_t batches);

        static void RecordGpuTime(float gpuMilliseconds);

        static void RecordFrame(float frameMilliseconds, float cpuMilliseconds);

        static void NextFrame();

        static const RenderStats& Get();

    private:
        static RenderStats sStats;
        static float sSmoothFrameMs;
        static float sSmoothCpuMs;
        static bool sHasFrameSamples;
        static bool sHasCpuSamples;
        static bool sHasGpuSamples;
    };

} // namespace golias
