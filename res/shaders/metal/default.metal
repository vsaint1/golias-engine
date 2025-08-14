#include <metal_stdlib>
using namespace metal;

// ======================
// Vertex Shader
// ======================
struct VertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float4 color    [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

struct VertexUniforms {
    float4x4 projection;
};

vertex VertexOut vertex_main(VertexIn in [[stage_in]],
                             constant VertexUniforms& uniforms [[buffer(1)]])
{
    VertexOut out;
    out.position = uniforms.projection * float4(in.position, 0.0, 1.0);
    out.texCoord = in.texCoord;
    out.color    = in.color;
    return out;
}

// ======================
// Fragment Shader
// ======================
struct FragmentUniforms {
    bool use_texture;
};

fragment float4 fragment_main(VertexOut in [[stage_in]],
                              texture2d<float> TEXTURE [[texture(0)]],
                              sampler samp [[sampler(0)]],
                              constant FragmentUniforms& uniforms [[buffer(0)]])
{
    if (uniforms.use_texture) {
           float4 tex_color = TEXTURE.sample(samp, in.texCoord);
           return tex_color * in.color;
       } else {
           return in.color;
       }
}
