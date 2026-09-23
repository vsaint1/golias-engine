@vertex

layout(location = 0) in vec3 aPosition;

layout(std140) uniform Skybox {
    mat4 Projection;
    mat4 ViewRotation;
    mat4 InverseProjection;
    mat4 InverseViewRotation;
    vec4 ViewportSize;
    vec4 SkyTint;
    vec4 GroundColor;
    vec4 SunDirection;
    vec4 Parameters;
    vec4 SunSettings;
};

out vec3 vSkyDirection;

void main() {

    vSkyDirection = aPosition;

    vec4 clipPosition = Projection * ViewRotation * vec4(aPosition, 1.0);

    gl_Position = vec4(clipPosition.xy, clipPosition.w, clipPosition.w);
}

@fragment

layout(std140) uniform Skybox {
    mat4 Projection;
    mat4 ViewRotation;
    mat4 InverseProjection;
    mat4 InverseViewRotation;
    vec4 ViewportSize;
    vec4 SkyTint;
    vec4 GroundColor;
    vec4 SunDirection;
    vec4 Parameters;   // x: atmosphereThickness, y: exposure, z: unused, w: unused
    vec4 SunSettings;  // x: celestialBrightness, y: celestialTextureAngularSize, z: starIntensity, w: unused
};

uniform samplerCube _Skybox;
uniform sampler2D _MainTexture; // sun/moon texture

in vec3 vSkyDirection;
out vec4 COLOR;

const vec3 kRayleighCoefficient = vec3(5.8, 13.5, 33.1) * 0.008;

// starfield hash gen
float hash_21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

vec3 ComputeStars(vec3 direction, float nightFactor) {
    if (nightFactor <= 0.001) return vec3(0.0);

    // star positions stay fixed relative to the sky rather than the screen.
    vec2 grid = direction.xz / (abs(direction.y) + 0.15) * 40.0;
    vec2 cell = floor(grid);
    float n = hash_21(cell);

    float star = step(0.9935, n) * (1.0 - smoothstep(0.0, 0.06, length(fract(grid) - 0.5)));
    float twinkle = 0.6 + 0.4 * sin(n * 634.0); // static per-star variation, no time input needed
    return vec3(star * twinkle * nightFactor);
}


vec3 compute_sky_color(vec3 direction, vec3 sunDir, float atmosphereThickness, out float nightFactor) {
    float altitude = clamp(direction.y, -1.0, 1.0);
    float aboveHorizon = max(altitude, 0.0);

    float sunAltitude = clamp(sunDir.y, -1.0, 1.0);            // signed, NOT clamped to 0
    float sunAboveHorizonClamped = max(sunAltitude, 0.0);

    float viewOpticalDepth = 1.0 / (aboveHorizon + 0.08);
    viewOpticalDepth *= mix(0.65, 1.35, atmosphereThickness);
    vec3 rayleighScatter = vec3(1.0) - exp(-kRayleighCoefficient * viewOpticalDepth);

    float sunOpticalDepth = 1.0 / (sunAboveHorizonClamped + 0.08);
    vec3 sunExtinction = exp(-kRayleighCoefficient * sunOpticalDepth * 0.5);
    vec3 sunsetTint = mix(vec3(1.0, 0.45, 0.2), vec3(1.0), sunAboveHorizonClamped);
    vec3 horizonTint = mix(sunsetTint, vec3(1.0), smoothstep(0.0, 0.35, sunAboveHorizonClamped));

    float cosTheta = dot(direction, sunDir);
    float rayleighPhase = 0.75 * (1.0 + cosTheta * cosTheta);

    float dayVisibility = smoothstep(-0.18, 0.05, sunAltitude);

    float horizonBlend = smoothstep(-0.12, 0.22, altitude);
    vec3 tintedZenith = SkyTint.rgb * vec3(0.72, 0.88, 1.15) * mix(horizonTint, vec3(1.0), horizonBlend);
    vec3 horizonColor = mix(GroundColor.rgb, tintedZenith, horizonBlend);

    // physical blue at night no matter what sunExtinction alone does.
    vec3 scatter = rayleighScatter * sunExtinction * rayleighPhase * 2.2 * dayVisibility;

    vec3 sky = horizonColor + scatter;
    sky *= mix(0.8, 1.15, smoothstep(0.0, 0.85, aboveHorizon));

    nightFactor = 1.0 - dayVisibility;
    float ambientFloor = 0.015; // small residual so night isn't pure black
    sky *= mix(1.0, ambientFloor, nightFactor);

    vec3 moonAmbient = vec3(0.05, 0.07, 0.12) * nightFactor * 0.5;
    sky += moonAmbient * aboveHorizon;

    return sky;
}

// Textured sun/moon sprite
vec3 sample_celestial_texture(vec3 direction, vec3 celestialDir, float celestialDot, float angularRadius) {
    vec3 upHint = (abs(celestialDir.y) > 0.99) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(upHint, celestialDir));
    vec3 up = cross(celestialDir, right);

    float localX = dot(direction, right);
    float localY = dot(direction, up);
    vec2 offset = vec2(localX, localY) / max(angularRadius, 0.001);
    vec2 uv = offset * 0.5 + 0.5;

    vec3 texel = texture(_MainTexture, uv).rgb;

    float edgeDist = length(offset);
    float mask = (1.0 - smoothstep(0.85, 1.0, edgeDist)) * step(0.0, celestialDot);

    return texel * mask;
}


void main() {

    vec3 direction = normalize(vSkyDirection);
    vec3 sunDir = (length(SunDirection.xyz) > 0.0001)
        ? normalize(SunDirection.xyz)
        : vec3(0.0, 1.0, 0.0);

    float atmosphereThickness = max(Parameters.x, 0.001);
    float exposure            = max(Parameters.y, 0.0);

    float celestialBrightness  = max(SunSettings.x, 0.0);
    float celestialAngularSize = (SunSettings.y > 0.0001) ? SunSettings.y : 0.045;
    float starIntensity        = (SunSettings.z > 0.0001) ? SunSettings.z : 0.6;

    float nightFactor;
    vec3 sky = compute_sky_color(direction, sunDir, atmosphereThickness, nightFactor);

    sky += ComputeStars(direction, nightFactor) * starIntensity;

    vec3 cubemapSky = texture(_Skybox, direction).rgb;
    sky = mix(sky, cubemapSky, 0.0);

 
    bool isDaytime = sunDir.y > 0.0;
    vec3 celestialDir = isDaytime ? sunDir : -sunDir;
    float celestialDot = max(dot(direction, celestialDir), 0.0);

    vec3 celestialTexture = sample_celestial_texture(direction, celestialDir, celestialDot, celestialAngularSize);

    float celestialAltitude = clamp(celestialDir.y, -1.0, 1.0);
    float celestialOpticalDepth = 1.0 / (max(celestialAltitude, 0.0) + 0.08);
    vec3 celestialTint = isDaytime
        ? exp(-kRayleighCoefficient * celestialOpticalDepth * 0.35)
        : vec3(0.85, 0.9, 1.0);


    float effectiveBrightness = isDaytime ? celestialBrightness : min(celestialBrightness, 0.35);

    sky += celestialTexture * celestialTint * effectiveBrightness;

    sky = max(sky, vec3(0.0)); 

    vec3 hdrColor = sky * exposure;

    float dither = (fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) / 255.0;

    COLOR = vec4(hdrColor + dither, 1.0);
}
