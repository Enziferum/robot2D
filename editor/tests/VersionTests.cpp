#include <iostream>
#include <memory>
#include <string>

#include <robot2D/Util/Logger.hpp>
#include <robot2D/Config.hpp>
#include <editor/BuildInfo.hpp>
#include <editor/Configuration.hpp>
#include <editor/ProjectManager.hpp>
#include <editor/serializers/ProjectSerializer.hpp>
#include <editor/EditorCache.hpp>
#include <yaml-cpp/yaml.h>

#include "TestSupport.hpp"

int main() {
    logger::Log::Init();
    try {
        tests::Directory fixture;
        editor::Configuration configuration;
        const auto [found, version] = configuration.getValue(editor::ConfigurationKey::Version);
        CHECK(found && version == editor::buildInfo::getVersion());
        CHECK(version == RB2_VERSION_FULL);
        CHECK(editor::buildInfo::getWindowTitle().find(version) != std::string::npos);
        CHECK(std::string(editor::buildInfo::gitSHA) == RB2_GIT_SHA);

        editor::ProjectManager manager(configuration);
        const editor::ProjectDescription description{"VersionFixture", fixture.path().u8string()};
        CHECK(manager.add(description));
        const auto projectPath = fixture / "VersionFixture.robot2D";
        auto projectData = YAML::Load(tests::read(projectPath));
        CHECK(projectData["Editor Version"].as<std::string>() == version);

        // Old metadata remains readable and is not treated as a schema version.
        projectData["Editor Version"] = "0.5";
        tests::write(projectPath, YAML::Dump(projectData));
        CHECK(manager.load(description));
        CHECK(manager.getCurrentProject() -> getEditorVersion() == "0.5");

        editor::EditorCache cache(configuration);
        const auto cachePath = fixture / "editor.cache";
        CHECK(cache.parseCache(cachePath.u8string()));
        const auto cacheData = YAML::Load(tests::read(cachePath));
        CHECK(cacheData["EditorVersion"].as<std::string>() == version);
        CHECK(cacheData["CacheVersion"].as<int>() == 1);
        std::cout << "Version consistency and legacy project loading passed: " << version << '\n';
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
