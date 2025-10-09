
out vec4 COLOR;

in vec3 NORMAL;
in vec3 WORLD_POSITION;
in vec2 UV;

uniform vec3 CAMERA_POSITION;

uniform sampler2D TEXTURE;

uniform vec3 LIGHT_POSITION;
uniform vec3 LIGHT_COLOR; // DEFAULT vec3(1.0, 1.0, 1.0)

struct CMaterial {
    vec3 albedo;
    vec3 specular;
    float shininess;
    vec3 emissive;
    // TODO: handle `maps` 
};

uniform CMaterial material;

uniform bool USE_TEXTURE;

void main() {

    vec3 color = USE_TEXTURE ? texture(TEXTURE, UV).rgb : material.albedo;

    const float ambient_strength = 0.1;

    vec3 ambient = ambient_strength * LIGHT_COLOR * color;
    vec3 norm = normalize(NORMAL);

    vec3 light_dir = normalize(LIGHT_POSITION - WORLD_POSITION);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * LIGHT_COLOR * color;

    COLOR = vec4(ambient + diffuse, 1.0);
}