void vertex() {
    vec3 pos = VERTEX;
    pos.z += sin(TIME * 2.0 + VERTEX.x * 0.1) * 0.1;

    gl_Position = VIEW_PROJECTION_MATRIX * vec4(pos, 1.0);
}

void fragment() {
    vec2 uv = UV;

    float pixelSize = 16.0;
    uv = floor(uv * pixelSize) / pixelSize;

    vec4 texColor = texture(TEXTURE, uv);

    float scanline = sin(UV.y * 300.0) * 0.1 + 0.9;

    ALBEDO = texColor.rgb * COLOR.rgb * scanline;
    ALPHA = texColor.a * COLOR.a;
}

