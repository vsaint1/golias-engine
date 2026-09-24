#include "render/passes/skybox_pass.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "graphics/texture_2d.h"
#include "graphics/texture_cube.h"
#include "render/frame_params.h"
#include "render/mesh.h"
#include "render/render_targets.h"

namespace golias {

    void SkySettings::ApplyTimeOfDay(float hour) {
        TimeOfDay   = hour;
        float t     = hour / 24.0f;
        float angle = t * glm::two_pi<float>() - glm::half_pi<float>();

        glm::vec3 dir;
        dir.x        = 0.0f;
        dir.y        = sinf(angle);
        dir.z        = cosf(angle);
        SunDirection = glm::normalize(dir);

        bool isDaytime = dir.y > 0.0f;
        SunBrightness  = isDaytime ? 0.6f : 0.15f;

        glm::vec3 nightTint = glm::vec3(0.02f, 0.03f, 0.08f);
        glm::vec3 duskTint  = glm::vec3(0.55f, 0.35f, 0.30f);
        glm::vec3 dayTint   = glm::vec3(0.22f, 0.50f, 1.00f);

        float elevation01 = glm::clamp(dir.y, -1.0f, 1.0f);
        float dayFactor   = glm::smoothstep(-0.15f, 0.15f, elevation01);
        float duskFactor  = 1.0f - glm::smoothstep(0.15f, 0.5f, elevation01);
        glm::vec3 blended = glm::mix(dayTint, duskTint, duskFactor);
        SkyTint           = glm::mix(nightTint, blended, dayFactor);
    }

    SkyboxPass::~SkyboxPass() {
        delete mSkyMesh;
        mSkyMesh = nullptr;
    }

    bool SkyboxPass::Setup() {
        mShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/skybox.gshader");
        if (!mShader) {
            GOLIAS_LOG_ERROR("Failed to load skybox shader");
            return false;
        }

        mSunTexture = Engine::GetInstance().GetAssetManager().Load<Texture2D>("golias/textures/Sun.jpg");
        if (!mSunTexture) {
            GOLIAS_LOG_ERROR("Failed to load sky sun texture: golias/textures/Sun.jpg");
            return false;
        }

        mMoonTexture = Engine::GetInstance().GetAssetManager().Load<Texture2D>("golias/textures/Moon.jpg");
        if (!mMoonTexture) {
            GOLIAS_LOG_ERROR("Failed to load sky moon texture: golias/textures/Moon.jpg");
            return false;
        }


        mSkyCubemap =
            TextureCube::CreateProcedural(mSettings.SkyTint, mSettings.GroundColor, mSettings.SunDirection, mSettings.SunBrightness);

        if (!mSkyCubemap) {
            GOLIAS_LOG_ERROR("Failed to create skybox cubemap");
            return false;
        }

        mShader->SetUniformBlockBinding(GpuLayout::SkyboxBlock, GpuLayout::SkyboxBinding);

        VertexLayout layout;
        layout.Elements = {
            {0, VertexFormat::Float3, 0}
        };
        layout.Stride = 3 * sizeof(float);

        const std::vector<float> vertices = {
            -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,
            1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
            1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
            -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,
        };

        mSkyMesh = new Mesh(layout, vertices);

        mSettings.ApplyTimeOfDay(12.0f);

        return true;
    }

    void SkyboxPass::Execute(FrameContext& ctx) {

        ctx.RenderTargets->GetHdrFramebuffer()->Bind();
        ctx.Device.SetViewport(ctx.Camera.Viewport);
        ctx.Device.SetScissorTestEnabled(false);
        ctx.Device.SetBlendMode(BlendMode::None);
        ctx.Device.SetDepthTestEnabled(false);
        ctx.Device.SetDepthWriteEnabled(false);
        ctx.Device.SetCullMode(CullMode::None);

        mShader->Bind();
        mShader->SetTexture(TextureSlots::MainTexture, mSettings.SunDirection.y > 0.0f ? mSunTexture.get() : mMoonTexture.get());
        mShader->SetTexture(TextureSlots::Skybox2, mSkyCubemap.get());
        mSkyboxParams.Update(ctx.Device, BuildGpuSkybox(ctx.Camera, mSettings));

        mSkyMesh->Bind();
        mSkyMesh->Draw();
        mSkyMesh->Unbind();

        ctx.Device.SetDepthTestEnabled(true);
        ctx.Device.SetDepthWriteEnabled(true);
        ctx.Device.SetCullMode(CullMode::Back);
    }

} // namespace golias
