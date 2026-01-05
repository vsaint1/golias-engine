layout(location = 0) in vec2 a_pos;

out vec2 v_uv;

uniform mat4 MODEL_MATRIX;
uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;

uniform vec2 TEXTURE_PIVOT;
uniform vec2 TEXTURE_SIZE;
uniform vec2 TEXTURE_UV_MIN;
uniform vec2 TEXTURE_UV_MAX;

void main() {
    
    vec2 local = (a_pos - TEXTURE_PIVOT ) * TEXTURE_SIZE;

    v_uv = mix(TEXTURE_UV_MIN, TEXTURE_UV_MAX, a_pos);
    
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(local, 0.0, 1.0);
}