
in vec3 v_texcoord;

out vec4 FRAG_COLOR;

uniform samplerCube SKYBOX;
uniform float EXPOSURE;

void main() {
    vec3 color = texture(SKYBOX, v_texcoord).rgb;
    
    color *= EXPOSURE;
    
    color = color / (color + vec3(1.0));
    
    color = pow(color, vec3(1.0 / 2.2));
    
    FRAG_COLOR = vec4(color, 1.0);
}
