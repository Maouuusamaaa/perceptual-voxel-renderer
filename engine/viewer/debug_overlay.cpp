#include "viewer/debug_overlay.hpp"
#include <sstream>
namespace pvr::viewer {std::string formatDebugOverlay(const ImportedRenderMetrics&m){std::ostringstream o;o<<"mode="<<(m.mode==RenderComparisonMode::Perceptual?"perceptual":"fixed")<<" chunks="<<m.importedChunks<<" blocks="<<m.importedBlocks<<" visible="<<m.visible<<" rejected="<<m.rejected<<" draws="<<m.drawCount<<" world_version="<<m.worldTruthVersion<<" backend="<<m.backend;if(!m.diagnostic.empty())o<<" diagnostic="<<m.diagnostic;return o.str();}}
