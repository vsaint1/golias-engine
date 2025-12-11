# Shader System Documentation

## Overview

The shader system provides a flexible way to create custom visual effects for your game using GLSL shaders. It's
compatible with OpenGL GLSL version 3.3 and OpenGL ES 3.0, making it suitable for both desktop and mobile platforms.

## Table of Contents

- [Getting Started](#getting-started)
- [Shader Structure](#shader-structure)
- [Built-in Variables](#built-in-variables)
- [Common Functions](#common-functions)
- [Custom Variables](#custom-variables)
- [Best Practices](#best-practices)
- [Examples](examples.md)

---

## Getting Started

### Loading a Shader

To load a shader in your game, use the rendering API:

```cpp
RID shader = rendering->create_shader_from_file("res/shaders/my_shader.glsl");
```

The shader file should be placed in your project's shader directory (typically `res/shaders/`).

### Basic Shader Template

Every shader consists of two main functions: `vertex()` and `fragment()`. Here's a minimal working shader:

### Shader Header

- shader_type canvas_item; // For 2D shaders
- shader_type spatial; // For 3D shaders
- #[compute]; // For Compute shaders

| Header                     | Type      | Description              |
  |----------------------------|-----------|--------------------------|
| `shader_type canvas_item;` | `2D`      | Used for 2D rendering    |
| `shader_type spatial;`     | `3D`      | Used for 3D rendering    |
| `#[compute]`               | `compute` | Used for compute shaders |



```glsl

shader_type canvas_item;

// Can be empty if not needed
void vertex() {

}

void fragment() {
    vec4 tex = texture(TEXTURE, UV);
    COLOR = tex.rgb * COLOR.rgb;
    ALPHA = tex.a * COLOR.a;
}
```

**What this does:**

- **Vertex Shader**: Transforms vertex positions to screen space
- **Fragment Shader**: Samples the texture and applies vertex colors

---

## Shader Structure

### Compute Shader

The compute shader is used for general-purpose computing tasks on the GPU. It operates independently of the graphics
pipeline.

> ⚠️ Note: Compute shaders are not supported on `Compatibility` renderer backend.

```glsl
#[compute]
#version 450

layout (local_size_x = 16, local_size_y = 16) in;

void main(){
// Do compute work here
}

```

### Vertex Shader

The vertex shader processes each vertex of your mesh.

```glsl

void vertex() {
    // You can also modify vertex positions, pass data to fragment shader, etc.
    VERTEX.z += sin(TIME * 2.0 + VERTEX.x * 0.1) * 0.1;
}
```

### Fragment Shader

The fragment shader runs for each pixel and determines its final color and transparency.

```glsl
void fragment() {
    // Set the pixel's color (RGB)
    COLOR = vec3(1.0, 0.0, 0.0);  // Red

    // Set the pixel's transparency (0.0 = transparent, 1.0 = opaque)
    ALPHA = 1.0;
}
```

---

## Built-in Variables

### Vertex Shader Variables

#### Inputs (Read-Only)

| Variable | Type   | Description                     |
|----------|--------|---------------------------------|
| `VERTEX` | `vec3` | Local vertex position (x, y, z) |
| `COLOR`  | `vec4` | Vertex color (RGBA)             |
| `UV`     | `vec2` | Texture coordinates (x, y)      |

#### Outputs

| Variable | Type | Description |
|----------|------|-------------|

### Fragment Shader Variables

#### Inputs (Read-Only)

| Variable | Type   | Description                      |
|----------|--------|----------------------------------|
| `UV`     | `vec2` | Interpolated texture coordinates |
| `COLOR`  | `vec4` | Interpolated vertex color        |

#### Outputs (Write to these)

| Variable | Type    | Default     | Description                   |
|----------|---------|-------------|-------------------------------|
| `COLOR`  | `vec3`  | `vec3(1.0)` | Base color of the pixel (RGB) |
| `ALPHA`  | `float` | `1.0`       | Transparency (0.0-1.0)        |

### Uniform Variables (Available in Both Shaders)

| Variable                 | Type        | Description                                |
|--------------------------|-------------|--------------------------------------------|
| `MODEL_MATRIX`           | `mat4`      | Transforms from local to world space       |
| `VIEW_MATRIX`            | `mat4`      | Camera view transformation                 |
| `PROJECTION_MATRIX`      | `mat4`      | Projection transformation                  |
| `VIEW_PROJECTION_MATRIX` | `mat4`      | Combined view × projection (optimized)     |
| `TIME`                   | `float`     | Elapsed time in seconds since start        |
| `CAMERA_POSITION`        | `vec3`      | World position of the camera               |
| `TEXTURE`                | `sampler2D` | The default texture sampler                |
| `SCREEN_SIZE`            | `vec2`      | Dimensions of the screen (width, height)   |
| `COLOR`                  | `vec4`      | Global color multiplier (RGBA)             |
| `VIEWPORT_SIZE`          | `vec2`      | Dimensions of the viewport (width, height) |

---

## Common Functions

### Mathematical Functions

```glsl
// Trigonometry
sin(x), cos(x), tan(x)
asin(x), acos(x), atan(y, x)

// Power and roots
pow(x, y)      // x^y
sqrt(x)        // Square root
exp(x)         // e^x
log(x)         // Natural logarithm

// Rounding and manipulation
abs(x)         // Absolute value
floor(x)       // Round down
ceil(x)        // Round up
fract(x)       // Fractional part (x - floor(x))
mod(x, y)      // Modulo (x % y)

// Range functions
min(x, y)              // Minimum value
max(x, y)              // Maximum value
clamp(x, min, max)     // Constrain x between min and max

// Interpolation
mix(a, b, t)           // Linear interpolation: a + (b-a) * t
step(edge, x)          // Returns 0.0 if x < edge, else 1.0
smoothstep(a, b, x)    // Smooth interpolation between a and b
```

### Vector Functions

```glsl
length(v)          // Vector length/magnitude
distance(a, b)     // Distance between two points
normalize(v)       // Normalize vector to length 1
dot(a, b)          // Dot product
cross(a, b)        // Cross product (vec3 only)
reflect(v, n)      // Reflect vector v around normal n
```

### Texture Functions

```glsl
texture(sampler, uv)        // Sample texture at UV coordinates
textureSize(sampler, lod)   // Get texture dimensions
```

### Additional Resources

- [GLSL Reference Card](https://www.opengl.org/sdk/docs/reference_card/opengl45-reference-card.pdf) - Complete list of
  built-in functions
- [Docs.gl](https://docs.gl/sl4/log) - Detailed OpenGL documentation

---

## Custom Variables

### Custom Uniforms

Uniforms allow you to pass data from your game code to the shader:

```glsl
uniform float myValue;
uniform vec3 myColor;
uniform sampler2D customTexture;

void fragment() {
    COLOR = myColor * myValue;
    ALPHA = 1.0;
}
```

**Setting uniforms from code:**

```cpp
 float time_value = SDL_GetTicks() / 1000.0f;
 rd->push_constant("myValue", &time_value, sizeof(float));
```

### Custom Varyings

Varyings pass data from the vertex shader to the fragment shader:

```glsl
varying vec3 worldPos;
varying vec2 customUV;

void vertex() {
    // Calculate and pass world position
    worldPos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    customUV = UV * 2.0;

}

void fragment() {
    // Use the interpolated world position
    float dist = length(worldPos - CAMERA_POSITION);
    COLOR = vec3(dist * 0.1) * texture(TEXTURE, customUV).rgb;
    ALPHA = 1.0;
}
```

**How varyings work:**

1. Set the value in the vertex shader for each vertex
2. GPU automatically interpolates values between vertices
3. Fragment shader receives the interpolated value for each pixel

---

## Best Practices

### Performance Tips

1. **Minimize texture lookups**: Each `texture()` call is expensive. Store results in variables if you need to use them
   multiple times.

```glsl
// Good
vec4 tex = texture(TEXTURE, UV);
COLOR = tex.rgb * 0.5;
ALPHA = tex.a;

// Bad
COLOR = texture(TEXTURE, UV).rgb * 0.5;
ALPHA = texture(TEXTURE, UV).a; // Samples texture twice!
```

2. **Use built-in functions**: Functions like `mix()`, `smoothstep()`, and `clamp()` are optimized by the GPU.

3. **Avoid branching**: `if` statements can be slow. Use `step()` or `smoothstep()` instead when possible.

```glsl
// Slower
if (UV.x > 0.5) {
COLOR = vec3(1.0);
} else {
COLOR = vec3(0.0);
}

// Faster
COLOR = vec3(step(0.5, UV.x));
```

### Debugging Tips

1. **Visualize values**: Output variables as colors to see what's happening

```glsl
COLOR = vec3(UV, 0.0); // Visualize UV coordinates
COLOR = vec3(fract(TIME)); // Visualize time
```

2. **Test incrementally**: Start with a simple shader and add complexity gradually

3. **Use color channels**: Split complex calculations across RGB channels for debugging

### Common Patterns

**Time-based animation:**

```glsl
float wave = sin(TIME * 2.0) * 0.5 + 0.5;  // Oscillates between 0 and 1
```

**Distance-based effects:**

```glsl
float dist = length(UV - vec2(0.5));  // Distance from center
```

**Tiling textures:**

```glsl
vec2 tiledUV = fract(UV * 4.0);  // Tile 4x4
```

---

## See Also

- [Shader Examples](tests/) - Shader examples can be found in the tests directory
- [GLSL Reference](https://www.opengl.org/sdk/docs/reference_card/opengl45-reference-card.pdf)
- [Docs.gl](https://docs.gl/) - OpenGL documentation

---

## Troubleshooting

### Common Issues

**"Shader failed to compile"**

- Check for syntax errors (missing semicolons, mismatched brackets)
- Ensure all variables are declared before use

**"Black screen / Nothing renders"**

- Make sure `ALPHA` is set to a value > 0
- Check that textures are loaded properly

**"Shader compiles but looks wrong"**

- Output intermediate values as colors for debugging
- Verify UV coordinates are in range [0, 1]
- Check that matrices are being multiplied in correct order