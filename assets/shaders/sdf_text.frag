in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;



// TODO: instead of changing 1 by 1 send the UBO instead
layout(std140) uniform TextEffect {
    // SHADOW
    int  shadow_enabled;
    vec4 shadow_color;
    vec2 shadow_uv_offset;

    // OUTLINE
    int outline_enabled;
    vec4 outline_color;
    float outline_thickness;

    // GLOW
    int glow_enabled;
    vec4 glow_color;
    vec2 glow_uv_offset;
};

struct Shadow {
    int enabled;
    vec4 color;
    vec2 uv_offset;
};

struct Outline {
    int enabled;
    vec4 color;
    float thickness;
};

// TODO: implement
struct Glow {
    int enabled;
};

uniform Shadow shadow;

uniform Outline outline;


void main() {
    const float sdf_step = 0.025;

    float distance = texture(Texture, TexCoord).r;

    float alpha_glyph = smoothstep(0.5 - sdf_step, 0.5 + sdf_step, distance);

    float alpha_outline = 0.0;

    if (outline.enabled != 0){
        alpha_outline = smoothstep(0.5 - sdf_step - outline.thickness, 0.5 - sdf_step, distance);
    }

    float alpha_shadow = 0.0f;

    if (shadow.enabled != 0){
        float shadow_distance = texture(Texture, TexCoord + shadow.uv_offset).r;
        alpha_shadow = smoothstep(0.5 - sdf_step, 0.5 + sdf_step, shadow_distance);
    }

    vec3 result_color = shadow.color.rgb * alpha_shadow;
    result_color = mix(result_color, outline.color.rgb, alpha_outline);
    result_color = mix(result_color, Color.rgb, alpha_glyph);

    float final_alpha = max(max(alpha_shadow, alpha_outline), alpha_glyph) * Color.a;

    FragColor = vec4(result_color, final_alpha);
}
