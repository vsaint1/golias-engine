out vec4 COLOR;

in vec3 NORMAL;
in vec3 WORLD_POSITION;
in vec2 UV;
in vec3 INSTANCE_COLOR;

uniform sampler2D TEXTURE;

// AMBIENT LIGHTING
uniform vec3 LIGHT_POSITION;
uniform vec3 LIGHT_COLOR; 

uniform vec3 CAMERA_POSITION; 

struct Metallic {
    vec3 specular;
    float value; 
};

struct Material {
    vec3 albedo;
    Metallic metallic;
    float roughness; 
    // TODO: handle textures later
};

uniform Material material;
uniform bool USE_TEXTURE;

// SIMPLIFIED PBR FRAGMENT SHADER (NEED TO HANDLE TEXTURES )
// OGLDEV: https://www.youtube.com/watch?v=XK_p2MxGBQs 
void main() {
    // ------------------------
    // Base color
    // ------------------------
    vec3 albedo = USE_TEXTURE ? texture(TEXTURE, UV).rgb : material.albedo;
    albedo *= INSTANCE_COLOR;

    // ------------------------
    // Geometry
    // ------------------------
    vec3 N = normalize(NORMAL);
    vec3 V = normalize(CAMERA_POSITION - WORLD_POSITION);
    vec3 L = normalize(LIGHT_POSITION - WORLD_POSITION);
    vec3 H = normalize(V + L); // Half vector

    // ------------------------
    // Ambient term (simplified)
    // ------------------------
    const float AMBIENT_INTENSITY = 0.2;
    vec3 ambient = AMBIENT_INTENSITY * LIGHT_COLOR * albedo;

    // ------------------------
    // Diffuse (Lambert)
    // ------------------------
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * albedo * LIGHT_COLOR;

    // ------------------------
    // Specular (Cook-Torrance microfacet)
    // ------------------------
    float roughness = clamp(material.roughness, 0.04, 1.0);
    float metallic = material.metallic.value;

    // Fresnel (Schlick)
    vec3 F0 = mix(vec3(0.04), material.metallic.specular, metallic);
    float HdotV = max(dot(H, V), 0.0);
    vec3  F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

    // Normal distribution (GGX)
    float NdotH = max(dot(N, H), 0.0);
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    float D = a2 / (3.14159 * denom * denom + 1e-5);

    // Geometry function (Smith-Schlick-GGX)
    float NdotV = max(dot(N, V), 0.0);
    float k = (roughness + 1.0);
    k = (k * k) / 8.0;
    float Gv = NdotV / (NdotV * (1.0 - k) + k);
    float Gl = NdotL / (NdotL * (1.0 - k) + k);
    float G = Gv * Gl;

    // Final specular BRDF
    vec3 numerator   = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 1e-5;
    vec3 specular = numerator / denominator;

    // Energy conservation: reduce diffuse when metallic increases
    vec3 kS = F;                // specular reflection
    vec3 kD = vec3(1.0) - kS;   // diffuse reflection
    kD *= 1.0 - metallic;       // no diffuse for metals

    // Combine lighting terms
    vec3 radiance = LIGHT_COLOR * NdotL;
    vec3 color = (kD * diffuse + specular) * radiance + ambient;

    // Tone mapping + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    COLOR = vec4(color, 1.0);
}