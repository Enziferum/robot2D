/*********************************************************************
   (c) Alex Raag 2026
   https://github.com/Enziferum
   robot2D - Zlib license.
   This software is provided 'as-is', without any express or
   implied warranty. In no event will the authors be held
   liable for any damages arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute
   it freely, subject to the following restrictions:
   1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.
   2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.
   3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#include <cmath>
#include <cstdint>
#include <robot2D/imgui/Api.hpp>
#include <editor/UIManager.hpp>
#include <editor/panels/ScenePanel.hpp>
#include <editor/panels/UtilPanel.hpp>
#include <editor/EditorConfig.hpp>
#include <imgui/imgui_internal.h>
#include "panels/RuntimeControls.hpp"
#include "panels/DockspaceHost.hpp"

namespace editor {

    namespace {
        constexpr ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        void resizeDockNodeReferences(ImGuiDockNode* node, ImVec2 size) {
            node -> SizeRef = size;
            auto* first = node -> ChildNodes[0];
            auto* second = node -> ChildNodes[1];
            if(!first || !second)
                return;
            auto axis = node -> SplitAxis;
            auto total = first -> SizeRef[axis] + second -> SizeRef[axis];
            auto ratio = total > 0.f ? first -> SizeRef[axis] / total : 0.5f;
            auto available = ImMax(size[axis] - ImGui::GetStyle().DockingSeparatorSize, 0.f);
            auto firstSize = size;
            auto secondSize = size;
            firstSize[axis] = available * ratio;
            secondSize[axis] = available - firstSize[axis];
            // Restored or hidden nodes can have Size=0; their SizeRef still contains the split ratio.
            resizeDockNodeReferences(first, firstSize);
            resizeDockNodeReferences(second, secondSize);
        }

        void drawDockNode(ImGuiDockNode* node) {
            auto flags = node -> IsLeafNode() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;
            if(ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<std::uintptr_t>(node -> ID)), flags,
                "0x%08X  %.0f x %.0f  (ref %.0f x %.0f)",
                node -> ID, node -> Size.x, node -> Size.y, node -> SizeRef.x, node -> SizeRef.y)) {
                for(auto* child: node -> ChildNodes) {
                    if(child)
                        drawDockNode(child);
                }
                ImGui::TreePop();
            }
        }
    }

    IUIManager::~IUIManager() = default;

    UIManager::UIManager(robot2D::Gui& gui) :
        m_gui{gui}, m_panels() {
    }

    bool UIManager::registerPanel(IPanel& panel, const std::string& key, const std::string& name,
        const std::string& windowName) {
        auto belongs = std::find_if(m_panels.begin(), m_panels.end(), [&panel](const auto& item) {
            return item.get() == &panel;
        });
        auto duplicate = std::find_if(m_registeredPanels.begin(), m_registeredPanels.end(), [&](const auto& item) {
            return item.key == key || item.panel == &panel;
        });
        if(key.empty() || belongs == m_panels.end() || duplicate != m_registeredPanels.end())
            return false;
        bool visible = panel.getState() == PanelState::Visible;
        panel.m_panelKey = key;
        EditorConfig::getConfig().panelVisibility.try_emplace(key, visible);
        m_registeredPanels.push_back({key, name, windowName.empty() ? "###" + key : windowName, &panel});
        return true;
    }

    bool UIManager::setPanelVisible(const std::string& key, bool visible) {
        auto found = std::find_if(m_registeredPanels.begin(), m_registeredPanels.end(), [&](const auto& item) {
            return item.key == key;
        });
        if(found == m_registeredPanels.end())
            return false;
        found -> panel -> setState(visible ? PanelState::Visible : PanelState::Closed);
        return true;
    }

    void UIManager::showAllPanels() {
        for(const auto& entry: m_registeredPanels)
            entry.panel -> setState(PanelState::Visible);
    }

    void UIManager::restoreDefaultLayout() {
        m_resetLayout = true;
    }

    bool UIManager::isStatusBarVisible() const {
        return std::any_of(m_panels.begin(), m_panels.end(), [](const auto& panel) {
            return panel -> getID() == UniqueType(typeid(UtilPanel)) && panel -> getState() == PanelState::Visible;
        });
    }

    void UIManager::renderDiagnostics() const {
        if(ImGui::BeginTable("Panels", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Panel");
            ImGui::TableSetupColumn("Visible");
            ImGui::TableSetupColumn("Dock node");
            ImGui::TableSetupColumn("Size");
            ImGui::TableHeadersRow();
            for(const auto& entry: m_registeredPanels) {
                auto* window = ImGui::FindWindowByName(entry.windowName.c_str());
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(entry.name.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(entry.panel -> getState() == PanelState::Visible ? "Yes" : "No");
                ImGui::TableNextColumn();
                if(window && window -> DockId)
                    ImGui::Text("0x%08X", window -> DockId);
                else
                    ImGui::TextDisabled("-");
                ImGui::TableNextColumn();
                if(window)
                    ImGui::Text("%.0f x %.0f", window -> Size.x, window -> Size.y);
                else
                    ImGui::TextDisabled("-");
            }
            ImGui::EndTable();
        }
        if(auto* root = ImGui::DockBuilderGetNode(m_dockspaceID))
            drawDockNode(root);
    }

    void UIManager::update(float dt) {
        for(const auto& panel: m_panels)
            panel -> update(dt);
    }

    void UIManager::render() {
        renderBaseCanvas();
    }

    void UIManager::setRuntimeControls(UIInteractor* interactor, robot2D::MessageBus& messageBus) {
        m_runtimeInteractor = interactor;
        m_runtimeMessageBus = &messageBus;
    }

    void UIManager::renderRuntimeToolbar() {
        if(!m_runtimeMessageBus)
            return;
        const bool running = m_runtimeInteractor && m_runtimeInteractor -> isRunning();
        const char* mode = !m_runtimeInteractor ? "No scene" : (running ? "Running" : "Editing");
        const auto& style = ImGui::GetStyle();
        const float buttonWidth = ImMax(ImGui::CalcTextSize(ICON_FA_STOP " Stop").x,
                ImGui::CalcTextSize(ICON_FA_PLAY " Play").x) + style.FramePadding.x * 2.f;
        const float modeWidth = ImGui::CalcTextSize(mode).x;
        const float gap = 16.f;
        const float height = ImGui::GetFrameHeight() + 8.f;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8.f, 4.f});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, running ? ImVec4{0.19f, 0.12f, 0.13f, 1.f}
                                                      : style.Colors[ImGuiCol_MenuBarBg]);
        const auto flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            | ImGuiWindowFlags_AlwaysUseWindowPadding;
        if(ImGui::BeginChild("##RuntimeToolbar", {0.f, height}, false, flags)) {
            const float available = ImGui::GetContentRegionAvail().x;
            const bool showMode = available >= buttonWidth + gap + modeWidth;
            const float groupWidth = buttonWidth + (showMode ? gap + modeWidth : 0.f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImMax(0.f, (available - groupWidth) * 0.5f));
            ImGui::PushID("Runtime");
            renderRuntimeControls(m_runtimeInteractor, *m_runtimeMessageBus, buttonWidth);
            ImGui::PopID();
            if(showMode) {
                ImGui::SameLine(0.f, gap);
                ImGui::AlignTextToFramePadding();
                if(running)
                    ImGui::TextColored({0.95f, 0.67f, 0.45f, 1.f}, "%s", mode);
                else
                    ImGui::TextDisabled("%s", mode);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    void UIManager::renderBaseCanvas() {
        auto flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport -> WorkPos);
        auto statusHeight = isStatusBarVisible() ? UtilPanel::getHeight() : 0.f;
        ImGui::SetNextWindowSize({viewport -> WorkSize.x, ImMax(1.f, viewport -> WorkSize.y - statusHeight)});
        ImGui::SetNextWindowViewport(viewport -> ID);
        robot2D::WindowOptions options{};
        options.flagsMask = flags;
        options.name = "Scene";
        robot2D::ScopedStyleVarF styles{{ImGuiStyleVar_WindowRounding, 0.f},
            {ImGuiStyleVar_WindowBorderSize, 0.f}};
        ui::renderDockspaceHost(options, "Robot2D_Dockspace", dockspaceFlags,
            BIND_CLASS_FN(canvasWindowFunction));
    }

    void UIManager::canvasWindowFunction() {
        renderRuntimeToolbar();
        if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            auto dockspaceID = ImGui::GetID("Robot2D_Dockspace");
            m_dockspaceID = dockspaceID;
            auto& config = EditorConfig::getConfig();
            if(m_resetLayout || !config.layoutCreated || !ImGui::DockBuilderGetNode(dockspaceID)) {
                createLayout();
                config.layoutCreated = true;
                if(m_resetLayout) {
                    m_resetLayout = false;
                    config.layoutSettings.clear();
                    ImGui::GetIO().WantSaveIniSettings = true;
                }
            }
            auto* root = ImGui::DockBuilderGetNode(dockspaceID);
            auto size = ImGui::GetContentRegionAvail();
            if(root && size.x > 0.f && size.y > 0.f
                && (std::abs(size.x - root -> Size.x) > 0.5f || std::abs(size.y - root -> Size.y) > 0.5f)) {
                resizeDockNodeReferences(root, size);
                ImGui::DockBuilderSetNodeSize(dockspaceID, size);
            }
            ImGui::DockSpace(dockspaceID, {0.f, 0.f}, dockspaceFlags);
            // Older layouts do not contain the settings panel yet.
            if(!ImGui::FindWindowSettingsByID(ImHashStr("###SceneSettingsPanel"))) {
                auto* inspector = ImGui::FindWindowSettingsByID(ImHashStr("###InspectorPanel"));
                if(inspector && inspector -> DockId && ImGui::DockBuilderGetNode(inspector -> DockId))
                    ImGui::DockBuilderDockWindow("###SceneSettingsPanel", inspector -> DockId);
            }
            if(!ImGui::FindWindowSettingsByID(ImHashStr("###ConsolePanel"))) {
                auto* assets = ImGui::FindWindowSettingsByID(ImHashStr("###AssetsPanel"));
                if(assets && assets -> DockId && ImGui::DockBuilderGetNode(assets -> DockId))
                    ImGui::DockBuilderDockWindow("###ConsolePanel", assets -> DockId);
            }
        }
        for(auto& panel: m_panels) {
            if(panel -> getState() == PanelState::Visible)
                panel -> render();
        }
    }

    void UIManager::createLayout() {
        auto rootID = ImGui::GetID("Robot2D_Dockspace");
        ImGui::DockBuilderRemoveNode(rootID);
        ImGui::DockBuilderAddNode(rootID, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(rootID, ImGui::GetContentRegionAvail());

        const auto& config = EditorConfig::getConfig();
        auto centerID = rootID;
        auto leftID = ImGui::DockBuilderSplitNode(centerID, ImGuiDir_Left, config.leftRatio,
                nullptr, &centerID);
        auto rightID = ImGui::DockBuilderSplitNode(centerID, ImGuiDir_Right,
                config.rightRatio / (1.f - config.leftRatio),
                nullptr, &centerID);
        auto assetsID = centerID;
        auto viewportID = ImGui::DockBuilderSplitNode(centerID, ImGuiDir_Up, config.centerTopRatio,
                nullptr, &assetsID);

        ImGui::DockBuilderDockWindow("###ScenePanel", leftID);
        ImGui::DockBuilderDockWindow("###ViewportPanel", viewportID);
        ImGui::DockBuilderDockWindow("###GamePanel", viewportID);
        ImGui::DockBuilderDockWindow("###AssetsPanel", assetsID);
        ImGui::DockBuilderDockWindow("###ConsolePanel", assetsID);
        ImGui::DockBuilderDockWindow("###AnimationPanel", assetsID);
        ImGui::DockBuilderDockWindow("###InspectorPanel", rightID);
        ImGui::DockBuilderDockWindow("###SceneSettingsPanel", rightID);
        ImGui::DockBuilderFinish(rootID);
    }

    void UIManager::blockEvents(bool flag) {
    }

    SceneEntity UIManager::getTreeItem(editor::UUID uuid) {
        return getPanel<ScenePanel>().getTreeItem(uuid);
    }
}
