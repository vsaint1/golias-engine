# Ember [![Build and Test](https://github.com/vsaint1/ember/actions/workflows/tests.yml/badge.svg)](https://github.com/vsaint1/ember/actions/workflows/tests.yml) [![cpp_20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard) ![license](https://img.shields.io/github/license/vsaint1/ember_engine.svg)



![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Android%20%7C%20iOS%20%7C%20Web-blue.svg)

**Ember** is a lightweight and modular **2D game engine/framework** written in **C/C++**, designed to be simple yet
powerful. It aims to provide developers with a flexible **_cross-platform_** solution to build 2Ds and
applications.

> ⚠️ **Note:** Currently there is no `Editor` and it is unlikely one will exist in the future.


---

## Engine Core Architecture

> ⚠️ **Note:**  This diagram is a work in progress and may not reflect the current state of the engine.
![Engine Architecture](docs/architecture.png)


---

## 📚 Documentation

- [Documentation](https://vsaint1.github.io/ember_engine)
- [Examples](https://github.com/vsaint1/ember_engine/tree/main/examples)  `Soon`
- [Tests](https://github.com/vsaint1/ember_engine/tree/main/tests)



## Building `WASM`

To build the Web version of the engine, you need to have [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) installed and activated.


```bash
git clone https://github.com/vsaint1/ember_engine.git
cd ember_engine

xmake f -p wasm
xmake build

python3 -m http.server -b 0.0.0.0 8080
```

Then open your browser and navigate to `http://localhost:8080/build/webgl/`

