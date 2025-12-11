shader_type canvas_item;

uniform float speed;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void vertex() {
}

void fragment() {
    float hue = fract(UV.x * 2.0 + UV.y + TIME * speed);
    vec3 rainbow = hsv2rgb(vec3(hue, 0.8, 1.0));

    COLOR.rgb *= rainbow;
    ALPHA = COLOR.a;
}

