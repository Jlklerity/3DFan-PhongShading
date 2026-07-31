#!/usr/bin/env sh
# =============================================================================
# build_web.sh – Build Assignment 3– 3DFan Phong Shading for WebGL using Emscripten
#
# Prerequisites:
#   1. Install the Emscripten SDK:  https://emscripten.org/docs/getting_started/
#   2. Activate it in your shell:   source /path/to/emsdk/emsdk_env.sh
#   3. Run this script from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
#
# Windowing backend (choose one):
#   GLFW  – default, recommended  (-DUSE_GLFW  -s USE_GLFW=3)
#   SDL2  – legacy fallback        (no -DUSE_GLFW, -s USE_SDL=2)
# =============================================================================

set -e

mkdir -p web

# --- Fetch GLM (same tag Desktop/Android pull via CMake FetchContent) -------
GLM_DIR="third_party/glm"
GLM_TAG="1.0.1"
 
if [ ! -d "$GLM_DIR" ]; then
    echo "Fetching GLM ${GLM_TAG} into ${GLM_DIR} ..."
    git clone --branch "$GLM_TAG" --depth 1 https://github.com/g-truc/glm.git "$GLM_DIR"
fi
 

# --- GLFW backend (default) -------------------------------------------------
emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Fan.cpp \
    Scene/Transform.cpp \
    Scene/SceneHUD.cpp \
    Scene/Scene3D.cpp \
    Scene/Ground.cpp \
    -IScene \
    -Ithird_party/glm \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

# --- SDL2 backend (opt-in) --------------------------------------------------
# To use SDL2 instead, comment out the GLFW block above and uncomment below:
# emcc -std=c++17 \
#     Scene/main.cpp \
#     Scene/Renderer.cpp \
#     Scene/Fan.cpp \
#     Scene/Transform.cpp \
#     -IScene \
#     -s USE_SDL=2 \
#     -s USE_WEBGL2=1 -s FULL_ES3=1 \
#     -s WASM=1 \
#     -s ALLOW_MEMORY_GROWTH=1 \
#     --preload-file android/app/src/main/assets/shader@assets/shader \
#     -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
