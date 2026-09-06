#include "render/render_stats.h"

namespace golias {

    namespace {

        constexpr float kEmaAlpha        = 0.05f;
        constexpr float kEmaInverseAlpha = 1.0f - kEmaAlpha;

        float smooth_sample(float sample, float previous, bool& hasSamples) {
            if (!hasSamples) {
                hasSamples = true;
                return sample;
            }

            return kEmaAlpha * sample + kEmaInverseAlpha * previous;
        }

    } // namespace

    RenderStats FrameStats::sStats    = {};
    float FrameStats::sSmoothFrameMs  = 0.0f;
    float FrameStats::sSmoothCpuMs    = 0.0f;
    bool FrameStats::sHasFrameSamples = false;
    bool FrameStats::sHasCpuSamples   = false;
    bool FrameStats::sHasGpuSamples   = false;

    void FrameStats::BeginFrame() {
        sStats.DrawCalls = 0;
        sStats.Batches   = 0;
        sStats.Vertices  = 0;
        sStats.Triangles = 0;
    }

    void FrameStats::RecordDrawCall(uint32_t vertices, uint32_t indices) {
        sStats.DrawCalls++;
        sStats.Vertices += vertices;
        sStats.Triangles += indices / 3;
    }

    void FrameStats::RecordCanvasBatch(uint32_t batches) {
        sStats.Batches += batches;
    }

    void FrameStats::RecordGpuTime(float gpuMilliseconds) {
        sStats.GpuTimeMs = smooth_sample(gpuMilliseconds, sStats.GpuTimeMs, sHasGpuSamples);
        sStats.GpuTiming = true;
    }

    void FrameStats::RecordFrame(float frameMilliseconds, float cpuMilliseconds) {
        sSmoothFrameMs = smooth_sample(frameMilliseconds, sSmoothFrameMs, sHasFrameSamples);
        sSmoothCpuMs   = smooth_sample(cpuMilliseconds, sSmoothCpuMs, sHasCpuSamples);
    }

    void FrameStats::NextFrame() {
        sStats.FrameTimeMs = sSmoothFrameMs;
        sStats.CpuTimeMs   = sSmoothCpuMs;
        sStats.Fps         = sSmoothFrameMs > 0.0f ? static_cast<uint32_t>(1000.0f / sSmoothFrameMs + 0.5f) : 0;
        sStats.FrameIndex++;
    }

    const RenderStats& FrameStats::Get() {
        return sStats;
    }

} // namespace golias
