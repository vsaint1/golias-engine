in vec2 tex_coord;
in vec4 color;

uniform sampler2D TEXTURE;
uniform bool use_texture;

uniform bool use_outline;
uniform bool use_shadow;
uniform vec4 outline_color;
uniform vec4 shadow_color;
uniform float outline_width;
uniform vec2 shadow_offset;
uniform vec2 texture_size;

out vec4 COLOR;

void main() {
    if (!use_texture) {
        COLOR = color;
        return;
    }

    vec4 tex_color = texture(TEXTURE, tex_coord);
    vec4 final_color = tex_color * color;

    if (!use_outline && !use_shadow) {
        COLOR = final_color;
        return;
    }

    vec2 texel_size = 1.0 / texture_size;

    if (use_shadow) {
        vec2 shadow_coord = tex_coord - shadow_offset * texel_size;
        vec4 shadow_sample = texture(TEXTURE, shadow_coord);

        if (tex_color.a < 0.1 && shadow_sample.a > 0.1) {
            COLOR = shadow_sample * shadow_color;
            return;
        }
    }

    if (use_outline && tex_color.a < 0.1) {
        float outline_alpha = 0.0;

        // 8-direction sampling for outline detection
        vec2 offsets[8] = vec2[](
        vec2(-outline_width, -outline_width), vec2(0.0, -outline_width), vec2(outline_width, -outline_width),
        vec2(-outline_width, 0.0),                                        vec2(outline_width, 0.0),
        vec2(-outline_width, outline_width),  vec2(0.0, outline_width),  vec2(outline_width, outline_width)
        );

        for (int i = 0; i < 8; i++) {
            vec2 sample_coord = tex_coord + offsets[i] * texel_size;
            float sample_alpha = texture(TEXTURE, sample_coord).a;
            outline_alpha = max(outline_alpha, sample_alpha);
        }

        if (outline_alpha > 0.1) {
            COLOR = outline_color;
            return;
        }
    }

    // Default: render the main texture
    COLOR = final_color;
}
