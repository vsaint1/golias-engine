
void vertex() {
    gl_Position = VIEW_PROJECTION_MATRIX * vec4(VERTEX, 1.0);
}

void fragment() {
    vec2 uv = UV;

    uv.x += sin(uv.y * 10.0 + TIME * 3.0) * 0.05;
    uv.y += cos(uv.x * 10.0 + TIME * 2.0) * 0.03;

    vec4 texColor = texture(TEXTURE, uv);
    ALBEDO = texColor.rgb * COLOR.rgb;
    ALPHA = texColor.a * COLOR.a;
}

