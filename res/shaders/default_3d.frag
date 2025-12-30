

in vec3 v_color;
in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_frag_pos;

out vec4 COLOR;

// PBR Textures
uniform sampler2D ALBEDO_TEXTURE;
uniform sampler2D METALLIC_TEXTURE;
uniform sampler2D ROUGHNESS_TEXTURE;
uniform sampler2D NORMAL_TEXTURE;
uniform sampler2D AO_TEXTURE;
uniform sampler2D EMISSIVE_TEXTURE;

// Texture flags (0 = not present, 1 = present)
uniform int HAS_ALBEDO;
uniform int HAS_METALLIC;
uniform int HAS_ROUGHNESS;
uniform int HAS_NORMAL;
uniform int HAS_AO;
uniform int HAS_EMISSIVE;

// Material properties
uniform vec4  BASE_COLOR;
uniform float METALLIC_FACTOR;
uniform float ROUGHNESS_FACTOR;
uniform vec3  EMISSIVE_FACTOR;

uniform vec3 VIEW_POSITION;  
uniform vec3 LIGHT_POSITION;
uniform vec3 LIGHT_COLOR;
uniform float AMBIENT_STRENGTH;
uniform float SPECULAR_STRENGTH;
uniform float SHININESS;

void main()
{
    // --- Material sampling ---
    vec4 albedo = (HAS_ALBEDO == 1)
        ? texture(ALBEDO_TEXTURE, v_texcoord)
        : BASE_COLOR;

    albedo.rgb *= v_color;

    float metallic  = (HAS_METALLIC  == 1) ? texture(METALLIC_TEXTURE,  v_texcoord).b : METALLIC_FACTOR;
    float roughness = (HAS_ROUGHNESS == 1) ? texture(ROUGHNESS_TEXTURE, v_texcoord).g : ROUGHNESS_FACTOR;
    float ao        = (HAS_AO        == 1) ? texture(AO_TEXTURE,        v_texcoord).r : 1.0;

    vec3 emissive = (HAS_EMISSIVE == 1)
        ? texture(EMISSIVE_TEXTURE, v_texcoord).rgb * EMISSIVE_FACTOR
        : vec3(0.0);

    // --- Normal ---
    vec3 norm = normalize(v_normal);

    if (HAS_NORMAL == 1) {
        vec3 tangentNormal = texture(NORMAL_TEXTURE, v_texcoord).xyz * 2.0 - 1.0;
        // TODO: apply TBN matrix
        norm = normalize(v_normal); // fallback for now
    }

    vec3 baseColor = albedo.rgb;

    // --- Lighting ---
    vec3 lightDir = normalize(LIGHT_POSITION - v_frag_pos);
    vec3 viewDir  = normalize(VIEW_POSITION - v_frag_pos);

    // Ambient (AO-affected)
    vec3 ambient = AMBIENT_STRENGTH * LIGHT_COLOR * ao;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * LIGHT_COLOR;

    // Specular (Blinn-Phong, roughness-weighted)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float adjustedShininess = max(SHININESS * (1.0 - roughness), 1.0);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), adjustedShininess);
    vec3 specular = SPECULAR_STRENGTH * spec * LIGHT_COLOR;

    // Metallic influence
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    specular = mix(specular, specular * baseColor, metallic);

    // Final color
    vec3 result = (ambient + diffuse + specular) * baseColor + emissive;

    COLOR = vec4(result, albedo.a);
}