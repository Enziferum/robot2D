#include "TestSupport.hpp"
#include <editor/AssetBrowserModel.hpp>
#include <editor/AssetPreviewCache.hpp>
#include <editor/EditorIcons.hpp>
#include <editor/panels/AssetsPanel.hpp>
#include <imgui/imgui_internal.h>
#include <cstring>
#include <editor/TaskQueue.hpp>
#include <robot2D/Util/Logger.hpp>
#include <iostream>
#include <cmath>

namespace {
    using namespace tests;

    template<typename Predicate>
    bool waitFor(Predicate predicate) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while(!predicate()) {
            if(std::chrono::steady_clock::now() >= deadline)
                return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return true;
    }

    void makeImage(const fs::path& path, unsigned int width, unsigned int height, unsigned char color) {
        fs::create_directories(path.parent_path());
        // TGA fixture: Image::save is intentionally not implemented in the engine.
        std::string bytes(18, '\0');
        bytes[2] = 2;
        bytes[12] = static_cast<char>(width & 255);
        bytes[13] = static_cast<char>(width >> 8);
        bytes[14] = static_cast<char>(height & 255);
        bytes[15] = static_cast<char>(height >> 8);
        bytes[16] = 32;
        bytes[17] = 0x28;
        for(unsigned int i = 0; i < width * height; ++i) {
            bytes.append(3, static_cast<char>(color));
            bytes += static_cast<char>(i % 2 == 0 ? 0 : 255);
        }
        write(path, bytes);
    }

    bool decoded(editor::AssetPreviewCache& cache, const fs::path& path) {
        return waitFor([&]() {
            editor::TaskQueue::GetQueue() -> process();
            auto* entry = cache.request(path);
            return entry && entry -> status == editor::PreviewStatus::Decoded;
        });
    }

    void navigation() {
        Directory fixture;
        const auto root = fixture / "assets";
        fs::create_directories(root / "Textures/sub");
        fs::create_directories(root / "Fonts");
        fs::create_directories(fixture / "outside");
        write(root / "z.txt", "x");
        editor::AssetBrowserModel model;
        CHECK(model.setRoot(root));
        CHECK(model.navigate(root / "Textures/sub"));
        CHECK(!model.navigate(fixture / "outside"));
        CHECK(!model.navigate(root / "z.txt"));
        CHECK(!model.navigate(root / ".."));
        const auto items = model.list(model.root()).entries;
        CHECK(items.size() == 3);
        CHECK(items[0].path().filename() == "Fonts");
        CHECK(items[1].path().filename() == "Textures");
        CHECK(items[2].path().filename() == "z.txt");
        fs::remove(root / "Textures/sub");
        CHECK(model.recoverCurrent());
        CHECK(model.current().filename() == "Textures");
        std::error_code error;
        fs::create_directory_symlink(fixture / "outside", root / "external", error);
        if(!error)
            CHECK(!model.navigate(root / "external"));
        CHECK(model.list(fixture / "outside").entries.empty());
        fs::create_directories(root / "New");
        model.refresh();
        CHECK(model.list(root).entries.size() >= 4);
        CHECK(!model.setRoot(root / "missing"));
        CHECK(model.current().empty());
    }

    void decode() {
        Directory fixture;
        const auto path = fixture / "wide.png";
        makeImage(path, 256, 64, 70);
        auto life = std::make_shared<int>(0);
        for(const auto* name: {"preview-alpha.png", "preview-rgb.jpg"}) {
            editor::AssetThumbnailTask fixtureTask(nullptr,
                fs::path(__FILE__).parent_path() / "res" / name, life);
            fixtureTask.execute();
            CHECK(fixtureTask.image().getSize().x == 16);
            CHECK(fixtureTask.image().getSize().y == 8);
            CHECK(fixtureTask.image().getColorFormat() == robot2D::ImageColorFormat::RGBA);
        }
        editor::AssetThumbnailTask task(nullptr, path, life);
        task.execute();
        const auto size = task.image().getSize();
        CHECK(size.x == 128 && size.y == 32);
        CHECK(task.image().getColorFormat() == robot2D::ImageColorFormat::RGBA);
        const auto& pixels = task.image().getBuffer();
        CHECK(pixels.size() == 128 * 32 * 4 && pixels[0] == 70);
        CHECK(pixels[3] >= 127 && pixels[3] <= 128);
        for(const auto extent: {robot2D::vec2u{1, 1}, robot2D::vec2u{1, 4096}, robot2D::vec2u{4096, 1}}) {
            makeImage(fixture / "thin.tga", extent.x, extent.y, 80);
            editor::AssetThumbnailTask thin(nullptr, fixture / "thin.tga", life);
            thin.execute();
            CHECK(thin.getOutcome() != editor::TaskOutcome::Failed);
            CHECK(thin.image().getSize().x > 0 && thin.image().getSize().y > 0);
            CHECK(thin.image().getSize().x <= 128 && thin.image().getSize().y <= 128);
        }
        write(fixture / "bad.png", "broken image");
        editor::AssetThumbnailTask bad(nullptr, fixture / "bad.png", life);
        bad.execute();
        CHECK(bad.getOutcome() == editor::TaskOutcome::Failed && !bad.getErrorMessage().empty());
        editor::AssetThumbnailTask stale(nullptr, path, life);
        life.reset();
        stale.execute();
        CHECK(stale.getOutcome() == editor::TaskOutcome::Cancelled);
        std::ofstream large(fixture / "large.png", std::ios::binary);
        large.seekp(17 * 1024 * 1024);
        large.put('x');
        large.close();
        life = std::make_shared<int>(0);
        editor::AssetThumbnailTask overLimit(nullptr, fixture / "large.png", life);
        overLimit.execute();
        CHECK(overLimit.getOutcome() == editor::TaskOutcome::Failed);
    }

    void cache() {
        Directory fixture;
        const auto root = fixture / "assets";
        makeImage(root / "a/same.png", 16, 8, 30);
        makeImage(root / "b/same.png", 8, 16, 200);
        makeImage(root / "third.png", 8, 8, 90);
        editor::AssetPreviewCache cache(2);
        cache.setRoot(fs::weakly_canonical(root));
        CHECK(cache.request(root / "a/same.png"));
        CHECK(cache.request(root / "b/same.png"));
        CHECK(!cache.request(root / "third.png"));
        CHECK(decoded(cache, root / "a/same.png"));
        CHECK(decoded(cache, root / "b/same.png"));
        CHECK(cache.request(root / "a/same.png") -> image.getBuffer()[4] == 30);
        CHECK(cache.request(root / "b/same.png") -> image.getBuffer()[4] == 200);
        cache.beginFrame();
        CHECK(cache.request(root / "third.png"));
        CHECK(cache.size() == 2);
        CHECK(decoded(cache, root / "third.png"));
        makeImage(root / "third.png", 8, 8, 120);
        fs::last_write_time(root / "third.png", fs::last_write_time(root / "third.png") + std::chrono::seconds(2));
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        cache.beginFrame();
        CHECK(cache.request(root / "third.png"));
        CHECK(decoded(cache, root / "third.png"));
        CHECK(cache.request(root / "third.png") -> image.getBuffer()[4] == 120);
        const auto second = fixture / "project2/assets";
        makeImage(second / "third.png", 8, 8, 240);
        cache.setRoot(fs::weakly_canonical(second));
        cache.beginFrame();
        CHECK(!cache.request(root / "third.png"));
        CHECK(cache.request(second / "third.png"));
        CHECK(decoded(cache, second / "third.png"));
        CHECK(cache.request(second / "third.png") -> image.getBuffer()[4] == 240);
        cache.clear();
        CHECK(cache.size() == 0);
        cache.beginFrame();
        CHECK(cache.request(second / "third.png"));
        // Drop the generation while CPU work may still be in flight; no callback owns the panel.
        cache.setRoot({});
        CHECK(waitFor([]() {
            editor::TaskQueue::GetQueue() -> process();
            return !editor::TaskQueue::GetQueue() -> hasPendingTasks();
        }));
    }

    class UIStub final : public editor::IUIManager {
    public:
        void blockEvents(bool) override {
        }
        editor::SceneEntity getTreeItem(editor::UUID) override {
            return {};
        }
    };

    void panelLayout() {
        Directory fixture;
        fs::create_directories(fixture / "assets/Textures");
        fs::create_directories(fixture / "assets/Scenes");
        auto* context = ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = {1200.f, 800.f};
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
        {
            robot2D::MessageBus bus;
            UIStub ui;
            editor::PrefabManager prefabs;
            editor::AssetsPanel panel(bus, nullptr, ui, prefabs);
            panel.setAssetsPath((fixture / "assets").u8string());
            panel.switchState(editor::AssetsPanel::State::Active);
            for(float panelWidth: {900.f, 360.f, 180.f}) {
                ImGui::NewFrame();
                ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_Always);
                ImGui::SetNextWindowSize({panelWidth, 400.f}, ImGuiCond_Always);
                panel.render();
                ImGuiWindow* tree = nullptr;
                ImGuiWindow* content = nullptr;
                for(auto* window: context -> Windows) {
                    if(std::strstr(window -> Name, "##FolderTree"))
                        tree = window;
                    if(std::strstr(window -> Name, "##AssetsContent")
                        && !std::strstr(window -> Name, "##AssetBrowser")
                        && !std::strstr(window -> Name, "##AssetPath"))
                        content = window;
                }
                CHECK(tree && content);
                CHECK(tree -> Size.x > 0.f && content -> Size.x > 0.f);
                CHECK(content -> Pos.x >= tree -> Pos.x + tree -> Size.x - 2.f);
                CHECK(content -> Pos.x + content -> Size.x <= panelWidth + 2.f);
                ImGui::Render();
                CHECK(ImGui::GetDrawData() -> TotalVtxCount > 0);
            }
            const auto frame = [&]() {
                ImGui::NewFrame();
                ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_Always);
                ImGui::SetNextWindowSize({900.f, 400.f}, ImGuiCond_Always);
                panel.render();
                ImGui::Render();
            };
            frame();
            frame();
            auto* main = ImGui::FindWindowByName("###AssetsPanel");
            CHECK(main);
            const float side = ImGui::GetFrameHeight();
            const auto click = [&](int index) {
                io.AddMousePosEvent(main -> DC.CursorStartPos.x
                    + index * (side + ImGui::GetStyle().ItemSpacing.x) + side * 0.5f,
                    main -> DC.CursorStartPos.y + side * 0.5f);
                frame();
                io.AddMouseButtonEvent(0, true);
                frame();
                io.AddMouseButtonEvent(0, false);
                frame();
            };
            const auto cards = [&]() {
                for(auto* window: context -> Windows) {
                    if(std::strstr(window -> Name, "##AssetBrowser"))
                        return ImGui::TableFindByID(window -> GetID("##AssetCards"));
                }
                return static_cast<ImGuiTable*>(nullptr);
            };
            CHECK(cards() && cards() -> ColumnsCount > 1);
            click(2);
            CHECK(cards() && cards() -> ColumnsCount == 1);
            click(1);
            CHECK(cards() && cards() -> ColumnsCount > 1);
            click(0);
            auto* layout = ImGui::TableFindByID(main -> GetID("##AssetsLayout"));
            CHECK(layout && layout -> ColumnsCount == 1);
            click(0);
            layout = ImGui::TableFindByID(main -> GetID("##AssetsLayout"));
            CHECK(layout && layout -> ColumnsCount == 2);
        }
        ImGui::DestroyContext(context);
    }

    void icons() {
        auto* context = ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = {800, 600};
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
        ImGui::NewFrame();
        auto* draw = ImGui::GetForegroundDrawList();
        for(int i = 0; i <= static_cast<int>(editor::EditorIcon::FolderTree); ++i) {
            for(float side: {16.f, 24.f, 48.f}) {
                const int first = draw -> VtxBuffer.Size;
                editor::drawEditorIcon(*draw, static_cast<editor::EditorIcon>(i), {50, 50}, side, IM_COL32_WHITE);
                CHECK(draw -> VtxBuffer.Size > first);
                for(int vertex = first; vertex < draw -> VtxBuffer.Size; ++vertex) {
                    const auto point = draw -> VtxBuffer[vertex].pos;
                    CHECK(std::isfinite(point.x) && std::isfinite(point.y));
                    CHECK(point.x >= 48.f && point.x <= 52.f + side);
                    CHECK(point.y >= 48.f && point.y <= 52.f + side);
                }
            }
        }
        ImGui::EndFrame();
        ImGui::DestroyContext(context);
    }
}

int main(int argc, char** argv) {
    logger::Log::Init();
    try {
        CHECK(argc == 2);
        const std::string name = argv[1];
        if(name == "navigation") navigation();
        else if(name == "decode") decode();
        else if(name == "cache") cache();
        else if(name == "icons") icons();
        else if(name == "panel") panelLayout();
        else CHECK(false && "Unknown asset browser scenario");
        std::cout << name << " passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
