# Android build path

This directory is the native Android build entry point for the PVR engine.
Use the Android NDK CMake toolchain from a host with the Android SDK/NDK installed.

Example configuration:

```sh
cmake -S android -B android/build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
cmake --build android/build
```

The current container does not include the Android SDK/NDK, so this pipeline is configuration-ready but not locally APK-packaged.

## Minecraft world viewer
The Android app is a standalone PVR viewer. It does not modify the official Minecraft client. Select a `.mcworld` document through Android Storage Access Framework; the app copies it to its cache and sends that path through the native reader/adapter. The first milestone targets `arm64-v8a`.
