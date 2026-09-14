#include "minecraft_android_bridge.hpp"
#include "minecraft/mcworld_reader.hpp"
#include "minecraft/minecraft_world_adapter.hpp"
#include "world/world_truth.hpp"
#include <string>
JNIEXPORT jint JNICALL Java_com_pvr_MinecraftWorldActivity_nativeImportWorld(JNIEnv* env, jclass, jstring path){
    if(!path) return -1;
    const char* raw=env->GetStringUTFChars(path,nullptr); if(!raw) return -2; std::string p(raw); env->ReleaseStringUTFChars(path,raw);
    auto read=pvr::minecraft::MinecraftWorldReader::read(p); if(!read.ok()) return -3;
    pvr::WorldTruth world(read.data.metadata.seed,read.data.metadata.sectionSize);
    auto imported=pvr::minecraft::MinecraftWorldAdapter::import(read.data,world,{});
    return imported.ok ? 0 : -4;
}
