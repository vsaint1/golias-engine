in vec2 TextureCoords;
in vec4 vColor;


uniform sampler2D TEXTURE;

out vec4 COLOR;


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

// 0 = none, 0.03f up
uniform float boldness;

void main() {
    const float sdf_step = 0.025;
    float distance = texture(TEXTURE, TextureCoords).r;

    float alpha_glyph = smoothstep(
        0.5 - sdf_step - boldness,
        0.5 + sdf_step + boldness,
        distance
    );

    float alpha_outline = 0.0;
    if (outline.enabled == 1) {
        alpha_outline = smoothstep(0.5 - sdf_step - outline.thickness, 0.5 - sdf_step, distance);
    }

    float alpha_shadow = 0.0;
    if (shadow.enabled == 1) {
        float shadow_distance = texture(TEXTURE, TextureCoords + shadow.uv_offset).r;
        alpha_shadow = smoothstep(0.5 - sdf_step, 0.5 + sdf_step, shadow_distance);
    }

    vec3 result_color = shadow.color.rgb * alpha_shadow;
    result_color = mix(result_color, outline.color.rgb, alpha_outline);
    result_color = mix(result_color, vColor.rgb, alpha_glyph);

    float final_alpha = max(max(alpha_shadow, alpha_outline), alpha_glyph) * vColor.a;
    COLOR = vec4(result_color, final_alpha);
}
