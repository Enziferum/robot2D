#pragma once
#include <filesystem>
#include <string>

namespace editor {
    struct ScriptProjectTools {
        std::filesystem::path python, generator, core, compiler, mono;
    };
    struct ScriptBuildResult {
        bool success{false};
        std::filesystem::path assembly;
        std::string diagnostic;
    };
    ScriptProjectTools scriptProjectTools();
    ScriptProjectTools scriptProjectTools(const std::filesystem::path& editorDirectory);
    ScriptBuildResult buildProjectScripts(const std::filesystem::path& root, const std::string& name,
        const ScriptProjectTools& tools);
    ScriptBuildResult generateProjectScripts(const std::filesystem::path& root, const std::string& name,
        const std::filesystem::path& sources, const ScriptProjectTools& tools);
}
