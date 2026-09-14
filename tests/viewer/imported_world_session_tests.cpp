#include "viewer/imported_world_session.hpp"
#include <cassert>
#include <iostream>
#include <filesystem>

static std::filesystem::path fixture(const char* name) {
    return std::filesystem::path(PVR_SOURCE_DIR) / "tests/fixtures/minecraft" / name;
}

int main() {
    using namespace pvr::viewer;
    ImportedWorldSession session;
    assert(session.load(fixture("minimal_supported.mcworld")));
    assert(session.importedChunkCount() == 2);
    assert(session.importedBlockCount() == 2);
    assert(session.worldSeed() == 424242);
    session.setMode(RenderComparisonMode::Perceptual);
    auto perceptual = session.render({{0,0,0},{0,0,1},90,1,0.1f,100}, 1);
    assert(perceptual.backend != "");
    assert(perceptual.visible + perceptual.rejected >= 1);
    session.setMode(RenderComparisonMode::FixedDistance);
    auto fixed = session.render({{0,0,0},{0,0,1},90,1,0.1f,100}, 1);
    assert(fixed.mode == RenderComparisonMode::FixedDistance);
    assert(fixed.worldTruthVersion == session.worldTruthVersion());
    assert(session.worldTruthVersion() > 0);

    ImportedWorldSession bad;
    assert(!bad.load(fixture("missing_terrain.mcworld")));
    assert(!bad.lastDiagnostic().empty());
    auto blocked = bad.render({{0,0,0},{0,0,1},90,1,0.1f,100}, 1);
    assert(blocked.visible == 0 && blocked.rejected == 0);

    std::cout << "pvr_minecraft_viewer_tests: PASS\n";
}
