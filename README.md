# aga engine

aga engine is a small c++23 game engine built around flecs modules. It uses GLFW for
windowing/input, wgpu-native for native WebGPU rendering, and GLM for math.

## layout

- `engine/core`: app loop, plugin interface, time, and exit resources
- `engine/window`: GLFW window resource
- `engine/input`: keyboard state resource
- `engine/transform`: local and global transforms
- `engine/camera`: perspective camera components and helpers
- `engine/render`: WebGPU renderer facade and mesh components
- `demo`: a small ECS scene with camera movement and rotating cubes

## build

```sh
nix develop
cmake --preset native-debug
cmake --build --preset native-debug
./build/native-debug/demo/aga_demo
```

For wasm:

```sh
nix develop
cmake --preset wasm-debug
cmake --build --preset wasm-debug
cd build/wasm-debug/demo
python3 -m http.server 8080
```

Open `http://127.0.0.1:8080/aga_demo.html` in a browser with WebGPU enabled.
The wasm preset uses Emscripten, GLFW's browser port, and the `emdawnwebgpu`
port. Native builds download the pinned wgpu-native release at configure time.
