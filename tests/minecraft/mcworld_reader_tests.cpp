#include "minecraft/mcworld_reader.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

namespace {
std::filesystem::path fixture(const char* name) {
    return std::filesystem::path(PVR_SOURCE_DIR) / "tests/fixtures/minecraft" / name;
}
}

int main() {
    using namespace pvr::minecraft;

    auto result = MinecraftWorldReader::read(fixture("minimal_supported.mcworld"));
    assert(result.ok());
    assert(result.data.metadata.worldName == "PVR Minecraft Fixture");
    assert(result.data.metadata.seed == 424242);
    assert(result.data.metadata.sectionSize == 16);
    assert(result.data.chunks.size() == 2);
    assert(result.data.chunks[0].x == -1);
    assert(result.data.chunks[0].z == 2);
    assert(result.data.chunks[0].sections.size() == 1);
    assert(result.data.chunks[0].sections[0].y == 0);
    assert(result.data.chunks[0].sections[0].blocks.size() == 2);
    assert(result.data.chunks[0].sections[0].blocks[0].sourceStateId == 5);
    assert(result.data.chunks[0].sections[0].blocks[1].sourceStateId == 7);
    assert(result.data.chunks[1].sections.empty());

    auto again = MinecraftWorldReader::read(fixture("minimal_supported.mcworld"));
    assert(again.ok());
    assert(result.data == again.data);

    auto malformed = MinecraftWorldReader::read(fixture("malformed.mcworld"));
    assert(!malformed.ok());
    assert(!malformed.diagnostics.empty());
    assert(malformed.diagnostics.front().severity == MinecraftDiagnosticSeverity::Error);

    auto missing = MinecraftWorldReader::read(fixture("missing_terrain.mcworld"));
    assert(!missing.ok());
    assert(!missing.diagnostics.empty());
    assert(missing.diagnostics.front().source == "pvr/terrain.bin");

    std::cout << "pvr_minecraft_reader_tests: PASS\n";
}
