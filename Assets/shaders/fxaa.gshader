@vertex

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPos, 1.0);
} 

@fragment

uniform sampler2D _MainTexture;

uniform float _TexelSizeX;
uniform float _TexelSizeY;
uniform float _SubpixelQuality;
uniform float _EdgeThreshold;
uniform float _EdgeThresholdMin;

out vec4 COLOR;

in vec2 vTexCoord;

float luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main() {
    vec2 texelSize = vec2(_TexelSizeX, _TexelSizeY);

    // Sample neighbourhood
    vec3 rgbM = texture(_MainTexture, vTexCoord).rgb;
    vec3 rgbNW = texture(_MainTexture, vTexCoord + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 rgbNE = texture(_MainTexture, vTexCoord + vec2(1.0, -1.0) * texelSize).rgb;
    vec3 rgbSW = texture(_MainTexture, vTexCoord + vec2(-1.0, 1.0) * texelSize).rgb;
    vec3 rgbSE = texture(_MainTexture, vTexCoord + vec2(1.0, 1.0) * texelSize).rgb;
    vec3 rgbN = texture(_MainTexture, vTexCoord + vec2(0.0, -1.0) * texelSize).rgb;
    vec3 rgbS = texture(_MainTexture, vTexCoord + vec2(0.0, 1.0) * texelSize).rgb;
    vec3 rgbW = texture(_MainTexture, vTexCoord + vec2(-1.0, 0.0) * texelSize).rgb;
    vec3 rgbE = texture(_MainTexture, vTexCoord + vec2(1.0, 0.0) * texelSize).rgb;

    float lumaM = luma(rgbM);
    float lumaN = luma(rgbN);
    float lumaS = luma(rgbS);
    float lumaW = luma(rgbW);
    float lumaE = luma(rgbE);
    float lumaNW = luma(rgbNW);
    float lumaNE = luma(rgbNE);
    float lumaSW = luma(rgbSW);
    float lumaSE = luma(rgbSE);

    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaW, lumaE)));
    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaW, lumaE)));
    float lumaRange = lumaMax - lumaMin;

    // Skip if contrast too low
    if(lumaRange < max(_EdgeThresholdMin, lumaMax * _EdgeThreshold)) {
        COLOR = vec4(rgbM, 1.0);
        return;
    }

    // Sub-pixel aliasing test
    float lumaL = (lumaN + lumaS + lumaW + lumaE) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0, (rangeL / lumaRange) - _SubpixelQuality);
    blendL = min(blendL / (1.0 - _SubpixelQuality), 1.0);
    blendL = blendL * blendL;

    // Edge direction
    float edgeH = abs(lumaN + lumaS - 2.0 * lumaM) * 2.0 +
        abs(lumaNE + lumaSE - 2.0 * lumaE) +
        abs(lumaNW + lumaSW - 2.0 * lumaW);

    float edgeV = abs(lumaW + lumaE - 2.0 * lumaM) * 2.0 +
        abs(lumaNW + lumaNE - 2.0 * lumaN) +
        abs(lumaSW + lumaSE - 2.0 * lumaS);

    bool isHorizontal = (edgeH >= edgeV);

    // Edge search
    float stepLength = isHorizontal ? texelSize.y : texelSize.x;
    float luma1 = isHorizontal ? lumaN : lumaW;
    float luma2 = isHorizontal ? lumaS : lumaE;
    float gradient1 = abs(luma1 - lumaM);
    float gradient2 = abs(luma2 - lumaM);
    bool is1Steepest = (gradient1 >= gradient2);

    float gradientScaled = 0.25 * max(gradient1, gradient2);
    float lumaLocalAvg;
    if(is1Steepest) {
        stepLength = -stepLength;
        lumaLocalAvg = 0.5 * (luma1 + lumaM);
    } else {
        lumaLocalAvg = 0.5 * (luma2 + lumaM);
    }

    vec2 currentUV = vTexCoord;
    if(isHorizontal) {
        currentUV.y += stepLength * 0.5;
    } else {
        currentUV.x += stepLength * 0.5;
    }

    // Walk along edge in both directions
    vec2 offset = isHorizontal ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);
    vec2 uv1 = currentUV - offset;
    vec2 uv2 = currentUV + offset;

    float lumaEnd1 = luma(texture(_MainTexture, uv1).rgb) - lumaLocalAvg;
    float lumaEnd2 = luma(texture(_MainTexture, uv2).rgb) - lumaLocalAvg;

    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;

    for(int i = 0; i < 12 && !(reached1 && reached2); ++i) {
        if(!reached1) {
            uv1 -= offset;
            lumaEnd1 = luma(texture(_MainTexture, uv1).rgb) - lumaLocalAvg;
            reached1 = abs(lumaEnd1) >= gradientScaled;
        }
        if(!reached2) {
            uv2 += offset;
            lumaEnd2 = luma(texture(_MainTexture, uv2).rgb) - lumaLocalAvg;
            reached2 = abs(lumaEnd2) >= gradientScaled;
        }
    }

    float dist1 = isHorizontal ? (vTexCoord.x - uv1.x) : (vTexCoord.y - uv1.y);
    float dist2 = isHorizontal ? (uv2.x - vTexCoord.x) : (uv2.y - vTexCoord.y);
    float distFinal = min(dist1, dist2);
    float edgeLen = dist1 + dist2;
    float pixelOffset = -distFinal / edgeLen + 0.5;

    bool isLumaCenterSmaller = lumaM < lumaLocalAvg;
    bool correctVariation1 = (lumaEnd1 < 0.0) != isLumaCenterSmaller;
    bool correctVariation2 = (lumaEnd2 < 0.0) != isLumaCenterSmaller;
    bool correctVariation = (dist1 < dist2) ? correctVariation1 : correctVariation2;
    float finalOffset = correctVariation ? pixelOffset : 0.0;
    finalOffset = max(finalOffset, blendL);

    vec2 finalUV = vTexCoord;
    if(isHorizontal) {
        finalUV.y += finalOffset * stepLength;
    } else {
        finalUV.x += finalOffset * stepLength;
    }

    COLOR = vec4(texture(_MainTexture, finalUV).rgb, 1.0);
}