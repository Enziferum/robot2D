#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "TestSupport.hpp"
#include <editor/SelectionService.hpp>
#include <editor/ConsoleLog.hpp>
#include <editor/EditorCache.hpp>
#include <editor/EditorConfig.hpp>
#include <editor/FileApi.hpp>
#include <editor/Messages.hpp>
#include <editor/serializers/YamlFile.hpp>
#include <robot2D/imgui/DragDropTarget.hpp>
#include <robot2D/Util/Logger.hpp>
#include <iostream>
#include <vector>

namespace {
    using namespace tests;

    void assetPaths() {
        Directory fixture;
        const auto root = fixture / "old-assets-parent/project";
        fs::create_directories(root / "assets/images");
        write(root / "assets/images/sheet.png", "image fixture");
        const auto reference = editor::projectAssetReference(root.u8string(),
                (root / "assets/images/sheet.png").u8string());
        CHECK(reference == "assets/images/sheet.png");
        CHECK(editor::projectAssetReference(root.u8string(), "assets\\images\\sheet.png") == reference);
        CHECK(editor::projectAssetReference(root.u8string(), "assets/images/../images/sheet.png") == reference);
        CHECK(editor::projectAssetReference(root.u8string(), "assets/missing.ttf") == "assets/missing.ttf");
        const auto moved = fixture / "moved project";
        fs::rename(root, moved);
        CHECK(read(fs::u8path(editor::resolveProjectAsset(moved.u8string(), reference))) == "image fixture");
        for(const auto& invalid: {"../outside.png", "assets/../../outside.png", "other-assets/file.png", ""}) {
            bool rejected = false;
            try { editor::resolveProjectAsset(moved.u8string(), invalid); }
            catch(const std::exception&) { rejected = true; }
            CHECK(rejected);
        }
        bool nulRejected = false;
        try { editor::projectAssetReference(moved.u8string(), std::string("assets/a\0b", 10)); }
        catch(const std::exception&) { nulRejected = true; }
        CHECK(nulRejected);
    }

    void assetImport() {
        Directory fixture;
        const auto root = fixture / "project";
        const auto source = fixture / "font.ttf";
        fs::create_directories(root);
        write(source, "original font");
        CHECK(editor::importProjectAsset(root.u8string(), source.u8string(),
            "assets/fonts") == "assets/fonts/font.ttf");
        CHECK(read(root / "assets/fonts/font.ttf") == "original font");
        CHECK(editor::importProjectAsset(root.u8string(), (root / "assets/fonts/font.ttf").u8string(),
            (root / "assets/fonts").u8string()) == "assets/fonts/font.ttf");
        write(source, "different font");
        bool rejected = false;
        try { editor::importProjectAsset(root.u8string(), source.u8string(), "assets/fonts"); }
        catch(const std::exception&) { rejected = true; }
        CHECK(rejected && read(root / "assets/fonts/font.ttf") == "original font");
        rejected = false;
        try { editor::importProjectAsset(root.u8string(), (fixture / "missing.png").u8string()); }
        catch(const std::exception&) { rejected = true; }
        CHECK(rejected && !fs::exists(root / "assets/missing.png"));
    }

    void assetPackage() {
        Directory fixture;
        const auto root = fixture / "project";
        const auto output = fixture / "game";
        fs::create_directories(root / "assets/fonts");
        fs::create_directories(root / "assets/enemies");
        write(root / "assets/fonts/hud.ttf", "font bytes");
        write(root / "assets/enemies/slime.prefab", "prefab bytes");
        write(root / "assets/enemies/idle.anim", "clip bytes");
        editor::copyProjectAssets(root.u8string(), output.u8string());
        fs::rename(root, fixture / "unavailable-source");
        CHECK(read(output / "assets/fonts/hud.ttf") == "font bytes");
        CHECK(read(output / "assets/enemies/slime.prefab") == "prefab bytes");
        CHECK(read(output / "assets/enemies/idle.anim") == "clip bytes");
        bool rejected = false;
        try { editor::copyProjectAssets(output.u8string(), output.u8string()); }
        catch(const std::exception&) { rejected = true; }
        CHECK(rejected);
        rejected = false;
        try { editor::copyProjectAssets(output.u8string(), (output / "assets/nested").u8string()); }
        catch(const std::exception&) { rejected = true; }
        CHECK(rejected && !fs::exists(output / "assets/nested"));
    }

    void selection() {
        editor::SelectionService selected;
        editor::UUID a{11}, b{22}, c{33};
        CHECK(selected.empty() && !selected.lastActive());
        selected.selectEntities({c, b, c, a}, b);
        CHECK(selected.getSelectedEntities() == std::vector<editor::UUID>({c, b, a}));
        CHECK(selected.lastActive() == b);
        selected.removeEntity(b);
        CHECK(selected.lastActive() == a);
        selected.selectEntities(selected.getSelectedEntities(), c);
        CHECK(selected.getSelectedEntities() == std::vector<editor::UUID>({c, a}));
        CHECK(selected.lastActive() == c);
        selected.toggleEntity(c);
        CHECK(!selected.hasEntity(c) && selected.lastActive() == a);
        selected.toggleEntity(b);
        CHECK(selected.hasEntity(b) && selected.lastActive() == b);
        selected.selectEntities({a, b}, c);
        CHECK(selected.lastActive() == b);
        selected.clear();
        CHECK(selected.empty() && !selected.lastActive());
        selected.selectEntity(editor::UUID{0});
        selected.removeEntity(editor::UUID{0});
        CHECK(selected.empty() && !selected.lastActive());
    }

    void consoleStream() {
        Directory fixture;
        const auto path = fixture / "source.log";
        editor::ConsoleLog log(path.u8string());
        log.poll();
        CHECK(!log.getStatus().empty());
        write(path, "[2026-10-15 12:00:00.123] [info] [thread 1] Editor: 100% ###\r\n"
            "[12:00:01] [warning] Editor: partial");
        log.poll();
        CHECK(log.getStatus().empty() && log.getEntries().size() == 1);
        CHECK(log.getEntries()[0].level == editor::LogLevel::Info);
        CHECK(log.getEntries()[0].text.back() != '\r');
        write(path, " complete\n", true);
        log.poll();
        CHECK(log.getEntries().size() == 2);
        CHECK(log.getEntries().back().level == editor::LogLevel::Warning);
        CHECK(log.getEntries().back().text.find("partial complete") != std::string::npos);
        const auto unchanged = read(path);
        log.clear();
        log.poll();
        CHECK(log.getEntries().empty() && read(path) == unchanged);
        const std::string utf8 = u8"[12:00:02] [error] Editor: Ошибка";
        write(path, utf8.substr(0, utf8.size() - 1), true);
        log.poll();
        CHECK(log.getEntries().empty());
        write(path, utf8.substr(utf8.size() - 1) + '\n', true);
        log.poll();
        CHECK(log.getEntries().size() == 1 && log.getEntries()[0].text == utf8);
        write(path, "short\n");
        log.poll();
        CHECK(log.getEntries().size() == 1 && log.getEntries()[0].text == "short");
        write(path, "replacement longer than before\n");
        log.poll();
        CHECK(log.getEntries().size() == 1 && log.getEntries()[0].text == "replacement longer than before");
        fs::rename(path, fixture / "old.log");
        log.poll();
        CHECK(!log.getStatus().empty());
        write(path, "new file\n");
        log.poll();
        CHECK(log.getEntries().size() == 1 && log.getEntries()[0].text == "new file");
    }

    void consoleLimits() {
        Directory fixture;
        const auto path = fixture / "source.log";
        editor::ConsoleLog log(path.u8string());
        write(path, std::string(100000, 'x') + '\n');
        log.poll();
        CHECK(log.getEntries().size() == 1 && log.getEntries()[0].text.size() < 66000);
        CHECK(log.getEntries()[0].text.find("[line truncated]") != std::string::npos);
        std::string lines;
        for(int i = 0; i < 20000; ++i)
            lines += std::to_string(i) + " line\n";
        write(path, lines);
        for(int i = 0; i < 4; ++i) log.poll();
        CHECK(log.getEntries().size() == 10000 && log.getEntries().back().text == "19999 line");
        lines.clear();
        for(int i = 0; i < 4000; ++i)
            lines += std::to_string(i) + ' ' + std::string(1024, 'a') + '\n';
        write(path, lines);
        for(int i = 0; i < 20; ++i) log.poll();
        std::size_t bytes = 0;
        for(const auto& entry: log.getEntries()) bytes += entry.text.size();
        CHECK(bytes <= 2 * 1024 * 1024 && log.getEntries().back().text.find("3999 ") == 0);
    }

    void yamlSave() {
        Directory fixture;
        const auto path = fixture / "saved.scene";
        YAML::Emitter document;
        document << YAML::BeginMap << YAML::Key << "Scene" << YAML::Value << "Saved" << YAML::EndMap;
        for(int i = 0; i < 20; ++i) CHECK(editor::yaml::writeFile(path.u8string(), document));
        const auto original = read(path);
        auto nulPath = path.u8string();
        nulPath.push_back('\0');
        CHECK(!editor::yaml::writeFile(nulPath, document));
        CHECK(!editor::yaml::writeFile((fixture / "missing/scene").u8string(), document));
        CHECK(read(path) == original);
        std::vector<fs::path> occupied;
        for(int i = 0; i < 128; ++i) {
            auto temporary = path;
            temporary += i == 0 ? ".tmp" : ".tmp." + std::to_string(i);
            write(temporary, "keep");
            occupied.push_back(temporary);
        }
        CHECK(!editor::yaml::writeFile(path.u8string(), document));
        CHECK(read(path) == original);
        for(const auto& temporary: occupied) CHECK(read(temporary) == "keep");
        fs::remove(occupied[3]);
        CHECK(editor::yaml::writeFile(path.u8string(), document));
        CHECK(!fs::exists(occupied[3]));
    }

    void payload() {
        robot2D::PayloadBuffer payload;
        payload.setData(nullptr, 0);
        CHECK(payload.unpack<std::string>().empty());
        std::string path = "scenes/Scene.scene";
        for(const auto size: {path.size(), path.size() + 1}) {
            payload.setData(path.data(), size);
            const auto decoded = payload.unpack<std::string>();
            CHECK(decoded == path && decoded.find('\0') == std::string::npos);
            std::vector<char> bytes(editor::StringBuffer::calcAllocSize(decoded));
            editor::Buffer packed(bytes.data());
            editor::pack_message_string(decoded, packed);
            editor::OpenSceneMessage message;
            message.unpack(bytes.data());
            CHECK(message.path == path);
        }
        std::string embedded("foo\0bar", 7);
        payload.setData(embedded.data(), embedded.size() + 1);
        CHECK(payload.unpack<std::string>() == embedded);
    }

    void configRoundtrip() {
        Directory fixture;
        auto& config = editor::EditorConfig::getConfig();
        CHECK(config.createDefaultConfig((fixture / "editor.ini").u8string()));
        config.cameraPos = {-21, 31};
        config.cameraSize = {1600, 900};
        config.panelVisibility["AssetsPanel"] = false;
        config.assetsThumbnailSize = 96;
        config.layoutCreated = true;
        config.layoutSettings = "[Window][100% ###]\nPath=C:\\sample\r\n";
        const auto layout = config.layoutSettings;
        CHECK(config.saveConfig((fixture / "editor.ini").u8string()));
        config.cameraPos = {0, 0};
        config.layoutSettings.clear();
        CHECK(config.loadConfig((fixture / "editor.ini").u8string()));
        CHECK(config.cameraPos == robot2D::vec2i(-21, 31));
        CHECK(config.cameraSize == robot2D::vec2i(1600, 900));
        CHECK(!config.panelVisibility.at("AssetsPanel") && config.assetsThumbnailSize == 96);
        CHECK(config.layoutCreated && config.layoutSettings == layout);
    }

    void configInvalid() {
        Directory fixture;
        auto& config = editor::EditorConfig::getConfig();
        write(fixture / "bad.ini", "[Editor]\nCameraPos=wrong\nCameraSize=bad\n"
            "[Assets]\nThumbnailSize=invalid\n[Layout]\nLeftRatio=nan\nRightRatio=5\nCenterTopRatio=-1\n");
        CHECK(config.loadConfig((fixture / "bad.ini").u8string()));
        CHECK(config.cameraPos == robot2D::vec2i(0, 0));
        CHECK(config.cameraSize == robot2D::vec2i(1280, 720));
        CHECK(config.assetsThumbnailSize == 64);
        CHECK(config.leftRatio == 0.11f && config.rightRatio == 0.12f && config.centerTopRatio == 0.8f);
        CHECK(!config.loadConfig((fixture / "absent.ini").u8string()));
        CHECK(!config.saveConfig((fixture / "missing/editor.ini").u8string()));
    }

    editor::ProjectDescription project(const Directory& fixture, const char* name) {
        editor::ProjectDescription result;
        result.name = name;
        result.path = (fixture / name).u8string();
        fs::create_directory(fs::u8path(result.path));
        return result;
    }

    void cacheRoundtrip() {
        Directory fixture;
        editor::Configuration defaults;
        editor::EditorCache cache(defaults);
        const auto path = fixture / "editor.cache";
        CHECK(cache.parseCache(path.u8string()));
        auto a = project(fixture, "Shooter");
        auto b = project(fixture, "Platformer");
        write(fs::u8path(a.path) / "keep.txt", "project data");
        CHECK(cache.addProject(a) && cache.addProject(b));
        CHECK(cache.setShowInspector(false));
        CHECK(cache.loadProject(cache.getProjects()[0]));
        CHECK(cache.getCurrentProject().name == a.name);
        CHECK(cache.parseCache(path.u8string()) && cache.parseCache(path.u8string()));
        CHECK(cache.getProjects().size() == 2 && !cache.isShowInspector());
        auto alias = a;
        alias.path = (fs::u8path(a.path) / ".." / "Shooter").u8string();
        CHECK(cache.addProject(alias) && cache.getProjects().size() == 2);
        CHECK(cache.removeProject(cache.getProjects()[0]));
        CHECK(read(fs::u8path(a.path) / "keep.txt") == "project data");
        CHECK(YAML::Load(read(path))["CacheVersion"].as<int>() == 1);
    }

    void cacheRecovery() {
        Directory fixture;
        editor::Configuration defaults;
        const std::vector<std::string> malformed = {"[broken", "", "[]",
                                                    "CacheVersion: 1\nProjects: {}\n",
                                                    "CacheVersion: nope\nProjects: []\n",
                                                    "CacheVersion: 1\nProjects: [{Name: MissingPath}]\n"};
        for(std::size_t i = 0; i < malformed.size(); ++i) {
            const auto path = fixture.path() / (std::to_string(i) + ".cache");
            write(path, malformed[i]);
            editor::EditorCache cache(defaults);
            CHECK(!cache.parseCache(path.u8string()));
            CHECK(cache.getError() == editor::EditorCacheError::InvalidCache);
            CHECK(cache.getProjects().empty() && cache.getCurrentProject().empty() && cache.isShowInspector());
            CHECK(read(fs::u8path(path.u8string() + ".invalid.bak")) == malformed[i]);
            CHECK(YAML::Load(read(path))["CacheVersion"].as<int>() == 1);
        }
    }

    void cacheFuture() {
        Directory fixture;
        const auto path = fixture / "future.cache";
        const std::string original = "CacheVersion: 999\nNewStructure: [leave, untouched]\n";
        write(path, original);
        editor::Configuration defaults;
        editor::EditorCache cache(defaults);
        CHECK(!cache.parseCache(path.u8string()));
        CHECK(cache.getError() == editor::EditorCacheError::UnsupportedVersion);
        CHECK(cache.addProject(project(fixture, "SessionOnly")));
        CHECK(cache.setShowInspector(false));
        CHECK(read(path) == original);
        CHECK(!fs::exists(fs::u8path(path.u8string() + ".invalid.bak")));
    }

    void cacheMigration() {
        Directory fixture;
        const auto a = project(fixture, "Shooter");
        const auto path = fixture / "old.cache";
        const std::string original = "Robot2D Editor: '0.5'\nShowInspector: false\nCurrentProject: '" + a.path +
            "'\nProjects:\n  - Name: Shooter\n    Path: '" + a.path +
            "'\n  - Name: Duplicate\n    Path: '" + a.path + "'\n";
        write(path, original);
        editor::Configuration defaults;
        editor::EditorCache cache(defaults);
        CHECK(cache.parseCache(path.u8string()));
        CHECK(cache.getProjects().size() == 1 && cache.getCurrentProject().name == a.name);
        CHECK(!cache.isShowInspector());
        CHECK(read(fs::u8path(path.u8string() + ".legacy.bak")) == original);
        CHECK(YAML::Load(read(path))["CacheVersion"].as<int>() == 1);
        CHECK(cache.parseCache(path.u8string()));
        CHECK(!fs::exists(fs::u8path(path.u8string() + ".legacy.bak.1")));
    }

#ifdef _WIN32
    void cacheWriteFailure() {
        Directory fixture;
        editor::Configuration defaults;
        editor::EditorCache cache(defaults);
        const auto path = fixture / "editor.cache";
        CHECK(cache.parseCache(path.u8string()));
        const auto original = read(path);
        const auto description = project(fixture, "NewProject");
        struct FileLock {
            HANDLE handle;
            ~FileLock() {
                if(handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
            }
        } lock{CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                   nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};
        CHECK(lock.handle != INVALID_HANDLE_VALUE);
        CHECK(!cache.addProject(description));
        CHECK(cache.getError() == editor::EditorCacheError::WriteFailed);
        CHECK(cache.getProjects().empty() && read(path) == original);
    }
#endif
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        const std::string name = argv[1];
        if(name == "selection") selection();
        else if(name == "asset-paths") assetPaths();
        else if(name == "asset-import") assetImport();
        else if(name == "asset-package") assetPackage();
        else if(name == "console-stream") consoleStream();
        else if(name == "console-limits") consoleLimits();
        else if(name == "yaml-save") yamlSave();
        else if(name == "payload") payload();
        else if(name == "config-roundtrip") configRoundtrip();
        else if(name == "config-invalid") configInvalid();
        else if(name == "cache-roundtrip") cacheRoundtrip();
        else if(name == "cache-recovery") cacheRecovery();
        else if(name == "cache-future") cacheFuture();
        else if(name == "cache-migration") cacheMigration();
#ifdef _WIN32
        else if(name == "cache-write-failure") cacheWriteFailure();
#endif
        else CHECK(false && "Unknown test case");
        std::cout << name << " passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
