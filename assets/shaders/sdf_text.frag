in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;

// TODO: create struct for each Font Effect

uniform vec4 OutlineColor;
uniform float OutlineThickness;

uniform vec4 ShadowColor;
uniform vec2 ShadowOffset;

void main() {
    const float sdf_step = 0.025;

    float distance = texture(Texture, TexCoord).r;

    float alpha_glyph = smoothstep(0.5 - sdf_step, 0.5 + sdf_step, distance);

    float alpha_outline = 0.0;
    if (OutlineThickness > 0.0) {
        alpha_outline = smoothstep(0.5 - sdf_step - OutlineThickness, 0.5 - sdf_step, distance);
    }

    float alpha_shadow = 0.0;
    if (length(ShadowOffset) > 0.0) {
        float shadow_distance = texture(Texture, TexCoord + ShadowOffset).r;
        alpha_shadow = smoothstep(0.5 - sdf_step, 0.5 + sdf_step, shadow_distance);
    }

    vec3 result_color = ShadowColor.rgb * alpha_shadow;
    result_color = mix(result_color, OutlineColor.rgb, alpha_outline);
    result_color = mix(result_color, Color.rgb, alpha_glyph);

    float final_alpha = max(max(alpha_shadow, alpha_outline), alpha_glyph) * Color.a;

    FragColor = vec4(result_color, final_alpha);
}
