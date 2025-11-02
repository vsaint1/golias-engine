in vec2 v_texCoord;
in vec4 v_color;
in vec2 v_position;

out vec4 COLOR;


uniform int DRAW_MODE; // 0=filled, 1=line, 2=text, 3=circle_filled, 4=circle_outline
uniform sampler2D TEXTURE;
uniform bool USE_TEXTURE;
uniform float LINE_THICKNESS;
uniform vec2 CIRCLE_CENTER;
uniform float CIRCLE_RADIUS;
uniform float CIRCLE_THICKNESS; // for outline

const float SMOOTHING = 0.5;

void main() {
    vec4 finalColor = v_color;
    
    // Mode 0: Filled shape (triangle, rect)
    if (DRAW_MODE == 0) {
        if (USE_TEXTURE) {
            vec4 texColor = texture(TEXTURE, v_texCoord);
            finalColor = texColor * v_color;
        }
    }
    // Mode 1: Line rendering (with thickness)
    else if (DRAW_MODE == 1) {

        finalColor = v_color;
    }
    // Mode 2: Text rendering
    else if (DRAW_MODE == 2) {
        if (USE_TEXTURE) {
            vec4 texColor = texture(TEXTURE, v_texCoord);
            finalColor = texColor * v_color;
        }
    }
    // Mode 3: Circle filled
    else if (DRAW_MODE == 3) {
        float dist = length(v_position - CIRCLE_CENTER);
        float alpha = 1.0 - smoothstep(CIRCLE_RADIUS - SMOOTHING, CIRCLE_RADIUS + SMOOTHING, dist);
        finalColor = vec4(v_color.rgb, v_color.a * alpha);
    }
    // Mode 4: Circle outline
    else if (DRAW_MODE == 4) {
        float dist = length(v_position - CIRCLE_CENTER);
        float innerRadius = CIRCLE_RADIUS - CIRCLE_THICKNESS * 0.5;
        float outerRadius = CIRCLE_RADIUS + CIRCLE_THICKNESS * 0.5;
        
        float outerAlpha = 1.0 - smoothstep(outerRadius - SMOOTHING, outerRadius + SMOOTHING, dist);
        float innerAlpha = smoothstep(innerRadius - SMOOTHING, innerRadius + SMOOTHING, dist);
        float alpha = outerAlpha * innerAlpha;
        
        finalColor = vec4(v_color.rgb, v_color.a * alpha);
    }
    
    if (finalColor.a < 0.01) {
        discard;
    }
    
    COLOR = finalColor;
}
