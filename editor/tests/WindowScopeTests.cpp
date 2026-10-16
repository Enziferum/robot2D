#include "TestSupport.hpp"
#include <iostream>
#include <type_traits>
#include <imgui/imgui_internal.h>
#include <robot2D/imgui/Api.hpp>
#include <editor/panels/IPanel.hpp>
#include "../src/panels/DockspaceHost.hpp"

namespace {
    using namespace tests;

    static_assert(!std::is_copy_constructible_v<robot2D::WindowScope>);
    static_assert(!std::is_move_constructible_v<robot2D::WindowScope>);

    struct Context {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ImGuiContext* context = ImGui::CreateContext();
        Context() {
            auto& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.DisplaySize = {800.f, 600.f};
            io.DeltaTime = 1.f / 60.f;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            unsigned char* pixels = nullptr;
            int width = 0, height = 0;
            io.Fonts -> GetTexDataAsRGBA32(&pixels, &width, &height);
        }
        ~Context() {
            ImGui::DestroyContext(context);
            ImGui::SetCurrentContext(previous);
        }
    };

    robot2D::WindowOptions options(const char* name) {
        robot2D::WindowOptions result;
        result.name = name;
        result.flagsMask = ImGuiWindowFlags_None;
        result.m_vars.emplace(ImGuiStyleVar_WindowPadding, robot2D::vec2f{13.f, 17.f});
        result.m_colors.emplace(ImGuiCol_Text, robot2D::Color::Red);
        return result;
    }

    struct Stacks {
        int windows = GImGui -> CurrentWindowStack.Size;
        int colors = GImGui -> ColorStack.Size;
        int variables = GImGui -> StyleVarStack.Size;
        ImVec2 padding = ImGui::GetStyle().WindowPadding;
        ImVec4 text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        void check() const {
            CHECK(GImGui -> CurrentWindowStack.Size == windows);
            CHECK(GImGui -> ColorStack.Size == colors);
            CHECK(GImGui -> StyleVarStack.Size == variables);
            CHECK(ImGui::GetStyle().WindowPadding.x == padding.x);
            CHECK(ImGui::GetStyle().WindowPadding.y == padding.y);
            auto color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
            CHECK(color.x == text.x && color.y == text.y && color.z == text.z && color.w == text.w);
        }
    };

    void windows() {
        Context context;
        for(int frame = 0; frame < 3; ++frame) {
            ImGui::NewFrame();
            Stacks before;
            int calls = 0;
            robot2D::createWindow(options("Visible"), [&]() {
                ++calls;
                ImGui::TextUnformatted("visible");
                CHECK(ImGui::GetStyle().WindowPadding.x == 13.f);
            });
            CHECK(calls == 1);
            before.check();
            ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
            ImGui::SetNextWindowSize({300.f, 200.f});
            robot2D::createWindow(options("Collapsed"), [&]() { ++calls; });
            CHECK(calls == 1);
            before.check();
            bool open = false;
            {
                robot2D::WindowScope closed{options("Closed"), &open};
                CHECK(!closed);
            }
            before.check();
            ImGui::EndFrame();
        }
    }

    void earlyReturn(const robot2D::WindowOptions& settings) {
        robot2D::WindowScope window{settings};
        if(window) {
            ImGui::TextUnformatted("early return");
            return;
        }
        throw std::runtime_error("Expected visible window");
    }

    void unwind() {
        Context context;
        ImGui::NewFrame();
        Stacks before;
        earlyReturn(options("Early"));
        before.check();
        bool caught = false;
        try {
            robot2D::createWindow(options("Throw"), []() {
                throw std::runtime_error("callback failure");
            });
        }
        catch(const std::runtime_error& error) {
            caught = std::string(error.what()) == "callback failure";
        }
        CHECK(caught);
        before.check();
        {
            robot2D::WindowScope outer{options("Outer")};
            Stacks inside;
            robot2D::createWindow(options("Inner"), []() { ImGui::TextUnformatted("nested"); });
            inside.check();
            CHECK(std::string(ImGui::GetCurrentWindow() -> Name) == "Outer");
        }
        before.check();
        ImGui::EndFrame();
        ImGui::NewFrame();
        robot2D::createWindow(options("AfterException"), []() { ImGui::Button("OK"); });
        ImGui::EndFrame();
    }

    class Panel final : public editor::IPanel {
    public:
        Panel() : IPanel(typeid(Panel)) {
        }
        int calls = 0;
        bool focused = false;
        void render() override {
            renderWindow(options("ScopePanel"), [this]() {
                ++calls;
                focused = ImGui::IsWindowFocused();
                ImGui::TextUnformatted("panel");
            });
        }
    };

    void panels() {
        Context context;
        Panel panel;
        ImGui::NewFrame();
        Stacks before;
        panel.setState(editor::PanelState::Closed);
        panel.render();
        CHECK(panel.calls == 0);
        before.check();
        panel.setState(editor::PanelState::Visible);
        panel.requestFocus();
        panel.render();
        CHECK(panel.calls == 1 && panel.focused);
        before.check();
        ImGui::EndFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
        panel.render();
        CHECK(panel.calls == 1);
        CHECK(panel.getState() == editor::PanelState::Visible);
        ImGui::EndFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
        panel.render();
        CHECK(panel.calls == 2);
        ImGui::EndFrame();
    }

    void docking() {
        Context context;
        ImGuiID dock = 0;
        ImGuiID left = 0;
        for(int frame = 0; frame < 7; ++frame) {
            ImGui::NewFrame();
            Stacks before;
            const bool hidden = frame >= 2 && frame <= 4;
            ImGui::SetNextWindowPos({0.f, 0.f});
            ImGui::SetNextWindowSize({700.f, 500.f});
            ImGui::SetNextWindowCollapsed(hidden, ImGuiCond_Always);
            bool called = false;
            editor::ui::renderDockspaceHost(options("DockHost"), "Dock", ImGuiDockNodeFlags_None, [&]() {
                called = true;
                dock = ImGui::GetID("Dock");
                if(frame == 0) {
                    ImGui::DockBuilderAddNode(dock, ImGuiDockNodeFlags_DockSpace);
                    ImGui::DockBuilderSetNodeSize(dock, {650.f, 400.f});
                    auto center = dock;
                    left = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Left, 0.3f, nullptr, &center);
                    ImGui::DockBuilderDockWindow("DockedPanel", left);
                    ImGui::DockBuilderFinish(dock);
                }
                ImGui::DockSpace(dock);
            });
            CHECK(called != hidden);
            before.check();
            {
                robot2D::WindowScope child{options("DockedPanel")};
                if(child)
                    ImGui::TextUnformatted("docked content");
            }
            auto* node = ImGui::DockBuilderGetNode(dock);
            CHECK(node && node -> LastFrameAlive == ImGui::GetFrameCount());
            CHECK(ImGui::FindWindowByName("DockedPanel") -> DockId == left);
            before.check();
            ImGui::EndFrame();
        }
    }
}

int main(int argc, char** argv) {
    try {
        CHECK(argc == 2);
        const std::string name = argv[1];
        if(name == "windows") windows();
        else if(name == "unwind") unwind();
        else if(name == "panels") panels();
        else if(name == "docking") docking();
        else CHECK(false && "Unknown window scope scenario");
        std::cout << name << " passed\n";
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
