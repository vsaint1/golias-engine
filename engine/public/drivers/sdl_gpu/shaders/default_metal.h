#pragma once



inline const char* default_vertex_metal = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UniformBlock
{
float4x4 VIEW_PROJECTION_MATRIX;
};

struct main0_out
{
float4 vColor [[user(locn0)]];
float2 vTexCoord [[user(locn1)]];
float4 gl_Position [[position]];
};

struct main0_in
{
float3 a_vertex [[attribute(0)]];
float4 a_color [[attribute(1)]];
float2 a_uv [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant UniformBlock& _30 [[buffer(0)]])
{
main0_out out = {};
out.vColor = in.a_color;
out.vTexCoord = in.a_uv;
out.gl_Position = _30.VIEW_PROJECTION_MATRIX * float4(in.a_vertex, 1.0);
return out;
})";

inline const char* default_fragment_metal = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
float4 FragColor [[color(0)]];
};

struct main0_in
{
float4 vColor [[user(locn0)]];
float2 vTexCoord [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> u_texture [[texture(0)]], sampler u_textureSmplr [[sampler(0)]])
{
main0_out out = {};
float4 tex = u_texture.sample(u_textureSmplr, in.vTexCoord);
if (tex.w < 0.100000001490116119384765625)
{
    discard_fragment();
}
out.FragColor = tex * in.vColor;
return out;
})";


