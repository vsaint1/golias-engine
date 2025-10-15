# Ember [![Build and Test](https://github.com/vsaint1/ember_engine/actions/workflows/tests.yml/badge.svg)](https://github.com/vsaint1/ember_engine/actions/workflows/tests.yml) [![docs](https://github.com/vsaint1/ember_engine/actions/workflows/docs.yml/badge.svg)](https://github.com/vsaint1/ember_engine/actions/workflows/docs.yml)  [![C++ 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard) ![License](https://img.shields.io/github/license/vsaint1/ember_engine.svg)

![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Android%20%7C%20iOS%20%7C%20Web-blue.svg)

**Ember** is a lightweight and modular **2D/3D game engine/framework** written in **C/C++**, designed to be **simple yet powerful**.  

> ⚠️ **Note:** Currently there is no `Editor` and one is unlikely to exist in the future.  
> Ember focuses on code-driven development, cross-platform compatibility, and learning.

---

## 🔹 Key Features

### 3D Features
- [x] **Model Loading** (OBJ, FBX, GLTF/GLB)
- [ ] **PBR (Physically Based Rendering)**
- [ ] **3D Physics** (coming soon)
- [ ] **Lighting System** (Directional, Point, Spot)
- [ ] **Shadow Mapping**
- [ ] **Post-Processing Effects** (Bloom, HDR, SSAO, Motion Blur, etc.)
- [ ] **Animation System** (Skeletal Animation)
- [ ] **Terrain System** (coming soon)

### 2D Features
- [x] **2D Sprite Rendering**
- [x] **Text/Shaping Rendering** (TrueType fonts and Emojis)
- [x] **Text Shaping** (HarfBuzz)
- [ ] **Tilemap Support** (Orthogonal, Isometric)
- [ ] **Audio System** (coming soon)
- [ ] **2D Physics**


### General Features
- [ ] **Audio System** (coming soon)
- [ ] **Particle System** (coming soon)
- [x] Cross-Platform **Rendering** and **API** by Design 
- [x] **Web (WASM) Support**
- [x] **Native Support:** Windows, Linux, macOS, Android, iOS
- [x] **ECS (Entity Component System) Based**
- [x] **Scripting Support** (Lua)
- [ ] **UI System** (Buttons, Inputs, Checkboxes, etc.)
- [ ] **Using Custom Shaders** (not implemented yet)

---


## 📱 Rendering Backends

| Platform                                                              | Type | Backend       | Status            |
| --------------------------------------------------------------------- | ---- | ------------- | ----------------- |
| Windows                                                               | 3D   | OpenGL 3.3    | ✅ Fully supported |
| Linux                                                                 | 3D   | OpenGL 3.3    | ✅ Fully supported |
| macOS                                                                 | 3D   | OpenGL 3.3    | ✅ Fully supported |
| macOS                                                                 | 3D   | Metal         | 🚧 Coming soon    |
| Android                                                               | 3D   | OpenGL ES 3.0 | ✅ Fully supported |
| iOS                                                                   | 3D   | OpenGL ES 3.0 | ✅ Fully supported |
| iOS                                                                   | 3D   | Metal         | 🚧 Coming soon    |
| Web                                                                   | 3D   | WebGL 3.0     | ✅ Fully supported |
| Windows, Linux, macOS, Android, iOS, Web, Nintendo, Playstation, Xbox | 2D   | Auto  | ✅ Fully supported |



## Engine Core Architecture

> ⚠️ **Note:** This diagram is a work in progress and may not reflect the current state of the engine.

![Engine Architecture](docs/architecture.png)

---

## 📚 Documentation & Examples

- [Official Documentation](https://vsaint1.github.io/ember_engine)
- [Examples](https://github.com/vsaint1/ember_engine/tree/main/examples) *(coming soon)*
- [Tests](https://github.com/vsaint1/ember_engine/tree/main/tests)

---

## 🎮 Games & Demos Created with Ember

| Game                 | Screenshot                          | Description                                   |
|----------------------|-------------------------------------|-----------------------------------------------|
| Flappy Bird          | ![Flappy](docs/2d_flappy.png)       | Simple Flappy Bird clone using Ember Engine (**old**) |
| Node Physics Example | ![Node Physics](docs/2d_node_phys.png) | 2D physics simulation (**old**)         |
| 3D Wireframe         | ![Wireframe](docs/3d_wireframe.png) | Basic 3D wireframe rendering demo            |
| Huge City            | ![City](docs/3d_map_huge.png)       | Large city scene rendering demo (Web)        |
| 3D Physics Example   | ![3D Physics](docs/3d_physics.png)  | 3D physics simulation with thousands of models |
| 2D Text Rendering    | ![2D Text](docs/2d_text.png)        | 2D text rendering with shaping (HarfBuzz)    |
---

## 🛠 Building for WASM

To build the Web version of the engine, you need [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) installed and activated.

```bash
git clone https://github.com/vsaint1/ember_engine.git
cd ember_engine


xmake f -p wasm
xmake build


python3 -m http.server -b 0.0.0.0 8080
