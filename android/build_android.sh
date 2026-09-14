#!/usr/bin/env bash
set -euo pipefail
: "${ANDROID_SDK_ROOT:=${ANDROID_HOME:-}}"
: "${ANDROID_NDK_ROOT:?Set ANDROID_NDK_ROOT to an installed Android NDK}"
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT (or ANDROID_HOME) to an installed Android SDK}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/android"
if [[ -x ./gradlew ]]; then ./gradlew :app:assembleDebug; else
  command -v gradle >/dev/null || { echo "Gradle is required (or provide gradle wrapper)." >&2; exit 2; }
  gradle :app:assembleDebug
fi
