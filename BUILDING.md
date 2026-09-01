
## Build Instructions

````markdown
## Build Instructions

Golias Engine uses CMake Presets to provide a consistent and reproducible build workflow across all supported platforms.

### 1. Common Setup

#### 1.1 Requirements
- CMake 3.25 or newer
- C++20 compatible compiler
- Git

#### 1.2 Repository Setup

```bash
git clone https://github.com/vsaint1/golias-engine.git
cd golias-engine
git submodule update --init --recursive
````

---

### 2. Desktop Platforms

### 2.1 Windows

#### 2.1.1 Requirements

* Windows 10 or newer
* Visual Studio 2022 (MSVC) or LLVM/Clang
* Vulkan SDK (optional)

#### 2.1.2 Build

Debug:

```bash
cmake --build --preset=windows-build --config Debug
```

Release:

```bash
cmake --build --preset=windows-build --config Release
```

---

### 2.2 Linux

#### 2.2.1 Requirements

* GCC or Clang with C++20 support
* X11 or Wayland development libraries
* Vulkan SDK (optional)

#### 2.2.2 Build

Debug:

```bash
cmake --preset=linux-debug
cmake --build build/linux/debug
```

Release:

```bash
cmake --preset=linux-release
cmake --build build/linux/release
```

---

### 2.3 macOS

#### 2.3.1 Requirements

* macOS 12 or newer
* Xcode Command Line Tools

#### 2.3.2 Build

Debug:

```bash
cmake --preset=macos-debug
cmake --build build/macos/debug
```

Release:

```bash
cmake --preset=macos-release
cmake --build build/macos/release
```

> ⚠️ Note: Metal is automatically selected when building on macOS.

---

### 3. Mobile Platforms

#### 3.1 Android

##### 3.1.1 Requirements

* Android SDK
* Android NDK r25 or newer
* Java 17 or newer

#### 3.1.2 Environment Variables

* ANDROID_HOME
* ANDROID_NDK_HOME

#### 3.1.3 Build

Here is the **correct replacement**, changing **only the Android build section** to reflect **Gradle / Android Studio usage**, while keeping the structure intact.

You can paste this directly over the Android build part.

````markdown
#### 3.1.3 Build

Android builds are generated and managed through **Gradle** and **Android Studio**.

1. Open the Android project in Android Studio:
   - `templates/android/` (or the Android project directory)

2. Ensure the correct NDK and SDK versions are configured.

3. Build from Android Studio **or** via Gradle CLI:

Debug:
```bash
./gradlew assembleDebug
````

See the official Android documentation for generating and managing signin
g keys:  
https://developer.android.com/studio/publish/app-signing#generate-key

Release:

```bash
./gradlew assembleRelease
```

> ⚠️Note: CMake is used internally by Gradle via the configured presets and toolchain files.


---

### 3.2 iOS

#### 3.2.1 Requirements

* macOS with Xcode 14 or newer
* iOS SDK
* Apple Developer account for device deployment

#### 3.2.2 Build

Debug:

```bash
cmake --preset=ios-debug
cmake --build build/ios/debug
```

Release:

```bash
cmake --preset=ios-release
cmake --build build/ios/release
```

Note: Code signing and provisioning must be configured in Xcode.

---

## 4. Web Platform

### 4.1 WebAssembly / WebGL

#### 4.1.1 Requirements

* Emscripten SDK

#### 4.1.2 Environment Setup

```bash
source emsdk_env.sh
```

#### 4.1.3 Build

Debug:

```bash
emcmake cmake --preset=web-debug
emmake cmake --build build/webgl/debug
```

Release:

```bash
emcmake cmake --preset=web-release
emmake cmake --build build/webgl/release
```


---