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

struct Glow {
    int enabled;
};

uniform Shadow shadow;
uniform Outline outline;
uniform float boldness;

void main() {
    const float sdf_width = 0.05;

    vec2 uv = TextureCoords;

    float distance = texture(TEXTURE, uv).r;

    // === SHADOW ===
    float alpha_shadow = 0.0;
    if (shadow.enabled == 1) {
        float shadow_distance = texture(TEXTURE, uv + shadow.uv_offset).r;
        alpha_shadow = smoothstep(0.5 - sdf_width, 0.5 + sdf_width, shadow_distance);
    }

    // === OUTLINE ===
    float alpha_outline = 0.0;
    if (outline.enabled == 1) {
        float outline_start = 0.5 - outline.thickness - sdf_width;
        float outline_end   = 0.5 - outline.thickness + sdf_width;
        alpha_outline = smoothstep(outline_start, outline_end, distance);
    }

    // === BOLDNESS ===
    float glyph_center = 0.5 - boldness;
    float alpha_glyph = smoothstep(glyph_center - sdf_width, glyph_center + sdf_width, distance);

    // === FINAL COLOR ===
    vec3 color = shadow.color.rgb * alpha_shadow;
    color = mix(color, outline.color.rgb, alpha_outline);
    color = mix(color, vColor.rgb, alpha_glyph);

    float final_alpha = max(max(alpha_shadow, alpha_outline), alpha_glyph) * vColor.a;
    COLOR = vec4(color, final_alpha);
}
