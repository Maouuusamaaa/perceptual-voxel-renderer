#pragma once
#include <jni.h>
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL Java_com_pvr_MinecraftWorldActivity_nativeImportWorld(JNIEnv*, jclass, jstring);
#ifdef __cplusplus
}
#endif
