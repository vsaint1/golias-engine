
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;
uniform vec2 TexelSize;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define THRESHOLD 0.05

struct Shadow_t {
    bool enabled;
    vec4 color;
    vec2 offset;
};

struct Outline_t {
    bool enabled;
    vec4 color;
    float thickness;
};

struct Glow_t {
    bool enabled;
    vec4 color;
    float strength;
    float radius;
};

uniform Shadow_t Shadow;
uniform Outline_t Outline;
uniform Glow_t Glow;

vec4 apply_shadow() {
    if (!Shadow.enabled) return vec4(0.0);

    vec4 tex_ = texture(Texture, TexCoord + Shadow.offset * TexelSize);
    if (tex_.a > THRESHOLD)
        return Shadow.color * tex_.a;

    return vec4(0.0);
}

vec4 apply_outline() {
    if (!Outline.enabled) return vec4(0.0);

    float outlineAlpha = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            if (x == 0 && y == 0) continue;
            vec2 offset = vec2(x, y) * Outline.thickness * TexelSize;
            outlineAlpha += texture(Texture, TexCoord + offset).a;
        }
    }
    outlineAlpha = clamp(outlineAlpha, 0.0, 1.0);
    return Outline.color * outlineAlpha;
}

vec4 apply_glow() {
    if (!Glow.enabled) return vec4(0.0);

    float glowAlpha = 0.0;
    int samples = 8;
    for (int i = 0; i < samples; ++i) {
        float angle = TWO_PI * float(i) / float(samples);
        vec2 offset = vec2(cos(angle), sin(angle)) * Glow.radius * TexelSize;
        glowAlpha += texture(Texture, TexCoord + offset).a;
    }
    glowAlpha /= float(samples);
    return Glow.color * glowAlpha * Glow.strength;
}

void main() {
    bool invalid_uv = TexCoord == vec2(0.0); 

    vec4 texColor = invalid_uv ? vec4(1.0) : texture(Texture, TexCoord);
    vec4 base = texColor * Color;

    if (invalid_uv && Color.a < THRESHOLD)
        discard;

    if (base.a < THRESHOLD) {
        vec4 shadow = apply_shadow();
        if (shadow.a > 0.0) {
            FragColor = shadow;
            return;
        }

        vec4 outline = apply_outline();
        if (outline.a > 0.0) {
            FragColor = outline;
            return;
        }

        vec4 glow = apply_glow();
        if (glow.a > 0.0) {
            FragColor = glow;
            return;
        }

        discard;
    }

    vec4 glow = apply_glow();
    FragColor = base + vec4(glow.rgb * glow.a, 0.0);
}
