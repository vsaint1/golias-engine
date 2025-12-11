shader_type canvas_item;

uniform float amplitude;
uniform float frequency;

void vertex() {
}

void fragment() {
    vec2 uv = UV;

    uv.x += sin(uv.y * amplitude + TIME * frequency) * 0.05;
    uv.y += cos(uv.x * amplitude + TIME * frequency) * 0.03;

    vec4 texColor = texture(TEXTURE, uv);
    COLOR.rgb *= texColor.rgb;
    ALPHA = texColor.a;
}

