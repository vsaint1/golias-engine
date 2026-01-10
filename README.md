# Golias Engine [![Build and Test](https://github.com/vsaint1/Golias_engine/actions/workflows/build.yml/badge.svg)](https://github.com/vsaint1/Golias_engine/actions/workflows/build.yml) [![docs](https://github.com/vsaint1/Golias_engine/actions/workflows/docs.yml/badge.svg)](https://github.com/vsaint1/Golias_engine/actions/workflows/docs.yml)  [![C++ 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard) ![License](https://img.shields.io/github/license/vsaint1/Golias_engine.svg)

![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Android%20%7C%20iOS%20%7C%20Web-blue.svg)

**Golias Engine** is a lightweight and modular **2D/3D game engine/framework** written in **C/C++**, designed to be *
*simple yet
powerful**.

> ⚠️ **Note:** Currently there is no `Editor` and one is unlikely to exist in the future.  
> Golias focuses on code-driven development, cross-platform compatibility, and learning.

---

## Rendering Backends & Supported Platforms



| Renderer Backend          | Status            |
|---------------------------|-------------------|
| OpenGL 3.3 / OpenGLES 3.0 | ✅ Fully supported |
| Metal                     | 🚫 No ETA          |
| Vulkan                    | 🚫 No ETA          |
| Direct3D 12               | 🚫 No ETA          |
| Proprietary APIs          | 🚫 No ETA         |

| Platform    | Category | Notes                      | Status |
|-------------|----------|----------------------------|--------|
| Windows     | Desktop  |                            | ✅      |
| Linux       | Desktop  |                            | ✅      |
| macOS       | Desktop  |                            | ✅      |
| Android     | Mobile   |                            | ✅      |
| iOS         | Mobile   |                            | ✅      |
| Web         | Web      | WebAssembly / WebGL        | ✅      |
| Xbox        | Console  | This platform requires NDA | 🚫     |
| PlayStation | Console  | This platform requires NDA | 🚫     |
| Nintendo    | Console  | This platform requires NDA | 🚫     |

> ⚠️ **Note:** Proprietary APIs for consoles are not publicly available and thus cannot be implemented or tested.

## Key Features

### 3D Features

- [x] **Model Loading** 
- [x] **PBR (Physically Based Rendering)** `SIMPLIFIED`
- [x] **Blinn-Phong Shading Model**
- [x] **Skybox Reflections** IBL (Image Based Lighting)
- [ ] Frustum Culling
- [x] **Normal mapping** (TBN calculated per fragment)
- [x] **3D Physics** (Bullet Physics)
- [x] **Lighting System**
    - [x] **Directional Light**
    - [ ] **Point Light**
    - [ ] **Spotlight**
- [x] **Shadow Mapping**
    - [ ] **CSM** (Cascaded Shadow Maps) for Directional Lights
- [ ] **Post-Processing Effects** (Bloom, HDR, SSAO, Motion Blur, etc.)
- [x] **Animation System** (Animation)
    - [x] Skeletal Animation **CPU** & **GPU** Skinning
    - [x] Hierarchical Animation
- [ ] **Level of Detail (LOD) Support**
- [x] **Skybox Support** (Cubemap -> 6 faces & Equirectangular)

### 2D Features

- [ ] **Tilemap Support** (Orthogonal, Isometric)
- [ ] **2D Physics** (Box2D)

### General Features

- [x] **Sprite Rendering**
- [x] **Text/Shaping Rendering** (TrueType fonts and Emojis)
- [x] **Audio System**
- [ ] **Particle System** (CPU & GPU)
- [x] Cross-Platform **Unified Rendering Pipeline** by Design
- [ ] **Custom Shader Language** based on `GLSL`
- [x] **Web (WASM) Support**
- [x] **Native Support:** Windows, Linux, macOS, Android, iOS and Web
- [x] **GameObject / Entity Component System (ECS)**
- [ ] **Scripting Support** (Lua)
- [ ] **UI Canvas System** (SCREEN/WORLD space) 
    - [x] Text
    - [ ] Button
    - [ ] Image
    - [ ] Slider
    - [ ] Panel
- [ ] **Networking Module** (ENET)
- [ ] **Batched Rendering** 
---

## Supported File Formats

| Asset Type | Supported Formats                              |
|------------|------------------------------------------------|
| 3D Models  | OBJ, GLTF, FBX, GLB                            |
| Images     | PNG, JPEG, BMP, TGA, DDS, ETC.                 |
| Fonts      | TTF & OTF.                                     |
| Audio      | OGG, WAV, FLAC, MP3, ETC.                      |
| Scenes     | JSON                                           |

## Engine Core Architecture

> ⚠️ **Note:** This diagram is a work in progress and may not reflect the current state of the engine.

![Engine Architecture](docs/architecture.png)

---

## 📚 Documentation & Examples

- [Official Documentation](https://vsaint1.github.io/golias-engine)
- [Examples](https://github.com/vsaint1/golias-engine/tree/master/examples)
- [Tests](https://github.com/vsaint1/golias-engine/tree/master/tests)

---

### Building from Source

Please refer to the [BUILDING.md](BUILDING.md) file for detailed instructions on how to build Golias Engine from source on various platforms.


## Third-Party Libraries Used

| Library | Description | License |
|--------|-------------|---------|
| **[SDL3](https://github.com/libsdl-org/SDL)** | Windowing, input/events handling, and cross-platform abstraction | Zlib License |
| **[GLM](https://github.com/g-truc/glm)** | Header-only mathematics library for graphics (vectors, matrices, quaternions) | MIT License |
| **[Bullet Physics](https://github.com/bulletphysics/bullet3)** | Real-time 3D physics simulation | Zlib License |
| **[cgltf](https://github.com/jkuhlmann/cgltf)** | Lightweight glTF 2.0 parsing library | MIT License |
| **[tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)** | Wavefront OBJ loader | MIT License |
| **[stb_image](https://github.com/nothings/stb)** | Header-only image loading (PNG, JPG, TGA, etc.) | Public Domain / MIT |
| **[miniaudio](https://github.com/mackron/miniaudio)** | Header-only audio playback and capture library | Public Domain / MIT |
| **[FreeType](https://github.com/freetype/freetype)** | Font rasterization engine | FreeType License (BSD-style) |
| **[nlohmann/json](https://github.com/nlohmann/json)** | Modern C++ JSON serialization/deserialization | MIT License |
| **[glad](https://github.com/Dav1dde/glad)** | OpenGL / OpenGL ES function loader | MIT License |

> All third-party libraries are vendored as git submodules or included directly in the `thirdparty/` directory.

# License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.