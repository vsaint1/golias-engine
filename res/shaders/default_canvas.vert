layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texcoord;
out vec4 v_color;
out vec2 v_uv;

uniform mat4 PROJECTION_MATRIX;
uniform mat4 VIEW_MATRIX;
uniform mat4 MODEL_MATRIX;
uniform vec3 CAMERA_POSITION;  
uniform bool USE_BILLBOARDING; 

void main(){
    v_uv = a_texcoord;
    v_color = a_color;
    
    if (USE_BILLBOARDING) {
       
        vec3 scaleVec = vec3(
            length(MODEL_MATRIX[0].xyz),
            length(MODEL_MATRIX[1].xyz),
            length(MODEL_MATRIX[2].xyz)
        );
        
        vec3 worldPos = (MODEL_MATRIX * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
        vec3 toCamera = normalize(CAMERA_POSITION - worldPos);
        
        vec3 up = vec3(0.0, 1.0, 0.0);
        vec3 right = normalize(cross(up, toCamera));
        up = cross(toCamera, right);
        
        vec3 billboardPos = worldPos + right * (a_pos.x * scaleVec.x) + up * (a_pos.y * scaleVec.y);
        gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * vec4(billboardPos, 1.0);
    } else {
        gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(a_pos, 1.0);
    }
}