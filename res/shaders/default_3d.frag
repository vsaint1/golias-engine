

in vec3 v_color;
in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_frag_pos;

out vec4 FRAG_COLOR;

// PBR Textures
uniform sampler2D ALBEDO_TEXTURE;
uniform sampler2D METALLIC_TEXTURE;
uniform sampler2D ROUGHNESS_TEXTURE;
uniform sampler2D NORMAL_TEXTURE;
uniform sampler2D AO_TEXTURE;
uniform sampler2D EMISSIVE_TEXTURE;

// Texture flags (bitwise)
const int HAS_ALBEDO_FLAG    = 1;  // 0x01
const int HAS_METALLIC_FLAG  = 2;  // 0x02
const int HAS_ROUGHNESS_FLAG = 4;  // 0x04
const int HAS_NORMAL_FLAG    = 8;  // 0x08
const int HAS_AO_FLAG        = 16; // 0x10
const int HAS_EMISSIVE_FLAG  = 32; // 0x20

uniform int TEXTURE_FLAGS;


struct Material {
    vec4  albedo;
    float metallic;
    float roughness;
    float ao;
    vec3  emissive;
    vec3  normal;
};

// Material properties uniform block
struct MaterialProperties {
    vec4  modulate;
    vec3  emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveStrength;
};

uniform MaterialProperties u_material;

// Directional light struct
struct DirectionalLight {
    vec3  direction;
    vec3  color;
    float intensity;
};

// Lighting configuration
const int MAX_DIRECTIONAL_LIGHTS = 32;
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directionalLightCount;

// Scene properties
uniform vec3 u_viewPosition;
uniform float u_ambientStrength;
uniform float u_specularStrength;
uniform float u_shininess;

Material SampleMaterial()
{
    Material mat;
    
    // Albedo
    mat.albedo = ((TEXTURE_FLAGS & HAS_ALBEDO_FLAG) != 0)
        ? texture(ALBEDO_TEXTURE, v_texcoord) * u_material.modulate
        : u_material.modulate;
    mat.albedo.rgb *= v_color;
    
    // Metallic
    mat.metallic = ((TEXTURE_FLAGS & HAS_METALLIC_FLAG) != 0)
        ? texture(METALLIC_TEXTURE, v_texcoord).b
        : u_material.metallicFactor;
    
    // Roughness
    mat.roughness = ((TEXTURE_FLAGS & HAS_ROUGHNESS_FLAG) != 0)
        ? texture(ROUGHNESS_TEXTURE, v_texcoord).g
        : u_material.roughnessFactor;
    
    // Ambient Occlusion
    mat.ao = ((TEXTURE_FLAGS & HAS_AO_FLAG) != 0)
        ? texture(AO_TEXTURE, v_texcoord).r
        : 1.0;
    
    // Emissive
    mat.emissive = ((TEXTURE_FLAGS & HAS_EMISSIVE_FLAG) != 0)
        ? texture(EMISSIVE_TEXTURE, v_texcoord).rgb * u_material.emissiveFactor * u_material.emissiveStrength
        : vec3(0.0);
    
    // Normal
    mat.normal = normalize(v_normal);
    if ((TEXTURE_FLAGS & HAS_NORMAL_FLAG) != 0) {
        vec3 tangentNormal = texture(NORMAL_TEXTURE, v_texcoord).xyz * 2.0 - 1.0;
        // TODO: Proper tangent-space to world-space transformation requires tangent/bitangent
        // For now, blend the normal map with the vertex normal
        mat.normal = normalize(mat.normal + tangentNormal * 0.5);
    }
    
    return mat;
}

void main()
{
    // --- Material sampling ---
    Material mat = SampleMaterial();
    vec3 baseColor = mat.albedo.rgb;

    // --- Lighting ---
    vec3 viewDir = normalize(u_viewPosition - v_frag_pos);
    
    // Ambient lighting (AO-affected)
    vec3 ambient = u_ambientStrength * baseColor * mat.ao;
    
    // Accumulate lighting from all directional lights
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    
    for (int i = 0; i < u_directionalLightCount && i < MAX_DIRECTIONAL_LIGHTS; ++i) {
        DirectionalLight light = u_directionalLights[i];
        vec3 lightDir = normalize(-light.direction);
        
        // Diffuse
        float diff = max(dot(mat.normal, lightDir), 0.0);
        diffuse += diff * light.color * light.intensity;
        
        // Specular (Blinn-Phong, roughness-weighted)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float adjustedShininess = max(u_shininess * (1.0 - mat.roughness), 1.0);
        float spec = pow(max(dot(mat.normal, halfwayDir), 0.0), adjustedShininess);
        specular += u_specularStrength * spec * light.color * light.intensity;
    }

    // Metallic influence on specular
    vec3 F0 = mix(vec3(0.04), baseColor, mat.metallic);
    specular = mix(specular, specular * baseColor, F0);

    // Final color
    vec3 result = (ambient + diffuse + specular) * baseColor + mat.emissive;

    FRAG_COLOR = vec4(result, mat.albedo.a);
}