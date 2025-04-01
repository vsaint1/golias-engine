# Ember [![build](https://github.com/vsaint1/ember_engine/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/vsaint1/ember_engine/actions/workflows/build.yaml/) [![docs](https://github.com/vsaint1/ember_engine/actions/workflows/docs.yaml/badge.svg?branch=main)](https://github.com/vsaint1/ember_engine/actions/workflows/docs.yaml)



**Ember** is a lightweight and modular **2D/3D game engine/framework** written in **C/C++**, designed to be simple yet powerful. It aims to provide developers with a flexible cross-platform solution to build 2D/3D games and applications.

> ⚠️ **Note:** Currently has no Editor.

### ✨ Features

- 🚀 **Cross-Platform**: 
  - Windows
  - Linux
  - macOS
  - Android
  - iOS
  - Web (via WebGL)

- 🎨 **Graphics Backend**:
  - OpenGL/ES (Windows, Linux, macOS, Android, Web)
  - Metal (macOS, iOS) **(coming soon)**

- 🛠️ **Lightweight Core**: Minimal dependencies for fast builds and portability.
- 🎮 **Input, Audio, and Texture Management**:  Simple and 
  extensible input, audio, and texture management.

### 📂 File Types

- 📸 **Image**: PNG, JPEG, WEBP, GIF, etc...

- 🔠 **Font**: TTF, SDF, etc... 

- 🔈 **Audio**: WAV, MP3, FLAC, OGG, etc...

- 🟥 **Model**: `TODO`

---

### 📦 Dependencies

- [sdl3](https://github.com/libsdl-org/SDL) (Windowing, Inputs, Events, Audio, ...)
- [stb_image](https://github.com/nothings/stb) (Image loader)
- [stb_truetype](https://github.com/nothings/stb) (TrueType/SDF font loader)
- [stb_vorbis](https://github.com/nothings/stb) (OGG Vorbis audio loader/decoder)
- [mini_audio](https://github.com/mackron/miniaudio) (Audio Engine backend)
- [glad](https://github.com/Dav1dde/glad) (OpenGL/ES Loader)
- [glm](https://github.com/g-truc/glm) (C++ math library)
- [tinyxml2](https://github.com/leethomason/tinyxml2) (XML Parser)

---

### 📱 Supported Platforms Overview

| Platform | Backend        | Status              |
|----------|----------------|---------------------|
| Windows  | OpenGL 3.3     | ✅ Fully supported  |
| Linux    | OpenGL 3.3     | ✅ Fully supported  |
| MacOS    | OpenGL 3.3     | ✅ Fully supported  |
| Android  | OpenGL ES 3.0  | ✅ Fully supported  |
| iOS      | OpenGL ES 3.0  | ✅ Fully supported  |
| Web      | WebGL 3.0      | ✅ Fully supported  |
| iOS      | Metal          | 🚧 Coming soon      |
| MacOS    | Metal          | 🚧 Coming soon      |

> ⚠️ **Note:** Metal backend is planned for future versions.

---

### 📚 Documentation & Examples

[Documentation](https://vsaint1.github.io/ember_engine)
[Examples](https://github.com/vsaint1/ember_engine/tree/main/examples)

---

### 🛠️ Build System

Ember Engine uses [cmake](https://cmake.org/) as its build system and supports the following toolchains:
- Visual Studio (Windows)
- GCC / Clang (Linux / macOS)
- Android NDK (Android)
- Xcode (macOS / iOS)
- Emscripten (WebGL)

---

### ⚠️ Disclaimer

This project was created for educational and personal purposes. It is **not** intended for commercial use.

---

### 📝 License

This project is distributed under the [MIT License](https://opensource.org/licenses/MIT).