#include "render/command_queue.h"

#include "core/engine.h"
#include "graphics/buffer.h"
#include "graphics/query.h"
#include "render/frame_params.h"
#include "render/render_pipeline.h"
#include "render/render_stats.h"
#include "render/render_targets.h"

namespace golias {

    CommandQueue::CommandQueue() {
    }

    CommandQueue::~CommandQueue() {

        delete mRenderTargets;
        mRenderTargets = nullptr;

        delete mPipeline;
        mPipeline = nullptr;
    }

    bool CommandQueue::Initialize() {
        mRenderTargets = new RenderTargets();
        mPipeline      = new RenderPipeline();

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        // Zero-filled dummy bone block, rebound once per frame ahead of any non-skinned draw.
        BufferDesc jointDesc = {
            .Target = BufferTarget::Uniform,
            .Usage  = BufferUsage::Static,
            .Size   = kMaxJoints * sizeof(glm::mat4),
        };

        mDefaultJointBuffer = device.CreateBuffer(jointDesc);
        if (!mDefaultJointBuffer) {
            return false;
        }

        if (!mPipeline->Initialize(mRenderTargets)) {
            return false;
        }

        if (device.IsQuerySupported()) {
            mActiveGpuQuery  = device.CreateQuery(QueryType::TimeElapsed);
            mStandbyGpuQuery = device.CreateQuery(QueryType::TimeElapsed);
        }

        return true;
    }

    void CommandQueue::Submit(const SpriteRenderCommand& command) {
        mSpriteCommands.push_back(command);
    }

    void CommandQueue::Submit(const RenderCommand& command) {
        mCommands.push_back(command);
    }

    void CommandQueue::Submit(const CameraCommand& command) {
        mCameraCommands.push_back(command);
    }

    void CommandQueue::Submit(const RenderCanvasCommand& command) {
        mCanvasCommands.push_back(command);
    }

    void CommandQueue::Submit(const LightCommand& command) {
        if (mLightCommands.size() >= kMaxLights) {
            return;
        }

        mLightCommands.push_back(command);
    }

    void CommandQueue::Submit(const UnlitCommand& command) {
        mUnlitCommands.push_back(command);
    }

    void CommandQueue::BeginFrame() {
        FrameStats::BeginFrame();
    }

    void CommandQueue::Execute() {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mStandbyGpuQuery) {
            uint64_t nanoseconds = 0;
            if (mStandbyGpuQuery->GetResult(&nanoseconds) == QueryResult::Available) {
                FrameStats::RecordGpuTime(nanoseconds * 1e-6f);
            }
        }

        if (mActiveGpuQuery) {
            mActiveGpuQuery->Begin();
        }

        // Sky ambient is uploaded with the lighting block so opaque materials receive
        // a baseline contribution even when no dynamic light reaches them.
        mLightingParams.Update(device, BuildGpuLighting(mLightCommands, glm::vec3(0.12f, 0.18f, 0.28f)));
        mDefaultJointBuffer->Bind(GpuLayout::JointsBinding);

        for (const CameraCommand& cameraCommand : mCameraCommands) {

            if (!mRenderTargets->EnsureHdrLdr(cameraCommand.Viewport)) {
                continue;
            }

            FrameContext ctx = {
                .Camera        = cameraCommand,
                .Frustum       = Frustum::FromMatrix(cameraCommand.Projection * cameraCommand.View),

                .RenderTargets = mRenderTargets,


                .Lights   = mLightCommands,
                .Commands = mCommands,
                .Sprites  = mSpriteCommands,
                .Canvas   = mCanvasCommands,
                .Unlit    = mUnlitCommands,

                .Device = device,
            };

            ctx.RenderTargets->GetHdrFramebuffer()->Bind();
            ctx.Device.SetViewport(cameraCommand.Viewport);
            ctx.Device.SetClearColor();
            ctx.Device.ClearBuffers(ClearFlag::Color | ClearFlag::Depth);

            mPipeline->Execute(ctx);
        }

        if (mActiveGpuQuery) {
            mActiveGpuQuery->End();
            std::swap(mActiveGpuQuery, mStandbyGpuQuery);
        }
    }

    void CommandQueue::EndFrame() {
        mCommands.clear();
        mCameraCommands.clear();
        mLightCommands.clear();
        mCanvasCommands.clear();
        mSpriteCommands.clear();
        mUnlitCommands.clear();
    }
} // namespace golias
