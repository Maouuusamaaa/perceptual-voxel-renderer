#pragma once
#include <string>
namespace pvr::minecraft {
enum class MinecraftDiagnosticSeverity { Warning, Error };
struct MinecraftDiagnostic { MinecraftDiagnosticSeverity severity{}; std::string source; std::string message; bool recoverable{}; };
}
