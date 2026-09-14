#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
test -f "$ROOT/android/app/src/main/java/com/pvr/MinecraftWorldActivity.java"
test -f "$ROOT/android/app/src/main/cpp/minecraft_android_bridge.cpp"
test -f "$ROOT/android/app/src/main/cpp/minecraft_android_bridge.hpp"
grep -q "arm64-v8a" "$ROOT/android/app/build.gradle"
grep -q "minecraft_android_bridge.cpp" "$ROOT/android/CMakeLists.txt"
grep -q "MinecraftWorldReader" "$ROOT/android/app/src/main/cpp/minecraft_android_bridge.cpp"
echo "android_import_smoke: PASS"
