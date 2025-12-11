shader_type canvas_item;

uniform float pixel_size;

void vertex() {
    VERTEX.z += sin(TIME * 2.0 + VERTEX.x * 0.1) * 0.1;

}

void fragment() {
    vec2 uv = UV;
    float pixelSize = pixel_size;
    uv = floor(uv * pixelSize) / pixelSize;

    vec4 texColor = texture(TEXTURE, uv);
    float scanline = sin(UV.y * 300.0) * 0.1 + 0.9;

    COLOR.rgb = texColor.rgb * scanline;
    ALPHA = texColor.a;
}