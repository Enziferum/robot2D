/*********************************************************************
(c) Alex Raag 2024
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

#include <robot2D/imgui/Api.hpp>
#include <editor/UIManager.hpp>
#include <editor/panels/ScenePanel.hpp>
#include <imgui/imgui_internal.h>

#include <editor/Enum.hpp>
#include <editor/EditorConfig.hpp>

namespace editor {

    namespace  {
        constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        DECLARE_ENUM(UIPanelType, ScenePanel, UtilsPanel, AssetsPanel,
                     ViewportPanel, GamePanel, AnimationPanel, InspectorPanel)
    }

    IUIManager::~IUIManager() = default;

    UIManager::UIManager(robot2D::Gui& gui):
        m_gui{gui}, m_panels() {}

    void UIManager::update(float dt) {
        for(const auto& panel: m_panels)
            panel -> update(dt);
    }

    void UIManager::render() {
        renderBaseCanvas();
    }

    void UIManager::renderBaseCanvas() {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport -> WorkPos);
        ImGui::SetNextWindowSize(viewport -> WorkSize);
        ImGui::SetNextWindowViewport(viewport -> ID);
        robot2D::WindowOptions dockWindowOptions{};

        {
            // TODO(a.raag): Scoped StyleVars
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.F);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.F);

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;


            dockWindowOptions.flagsMask = window_flags;
            dockWindowOptions.name = "Scene";
            ImGui::PopStyleVar(2);
        }


        robot2D::createWindow(dockWindowOptions, BIND_CLASS_FN(canvasWindowFunction));
    }

    void UIManager::canvasWindowFunction() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("Robot2D_Dockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),dockspace_flags);

            auto& config = EditorConfig::getConfig();
            if(!config.layoutCreated) {
                createLayout();
                config.layoutCreated = true;

                std::fstream file("robot2D.ini", std::ios::out | std::ios::ate);
                std::size_t iniOutSize = 0;
                auto iniData = ImGui::SaveIniSettingsToMemory(&iniOutSize);

                file.write(iniData, iniOutSize);
                file.close();
            }
        }

        for(auto& panel: m_panels)
            panel -> render();
    }


    void UIManager::createLayout() {

        std::unordered_map<UIPanelType, std::string> windowIDS = {
                {UIPanelType::ScenePanel, "###" + ENUM2STR(UIPanelType::ScenePanel)},
                {UIPanelType::UtilsPanel, "###" + ENUM2STR(UIPanelType::UtilsPanel)},
                {UIPanelType::AssetsPanel, "###" + ENUM2STR(UIPanelType::AssetsPanel)},
                {UIPanelType::ViewportPanel, "###" + ENUM2STR(UIPanelType::ViewportPanel)},
                {UIPanelType::GamePanel, "###" + ENUM2STR(UIPanelType::GamePanel)},
                {UIPanelType::AnimationPanel, "###" + ENUM2STR(UIPanelType::AnimationPanel)},
                {UIPanelType::InspectorPanel, "###" + ENUM2STR(UIPanelType::InspectorPanel)},
        };


        ImGuiID dockspace_id = ImGui::GetID("Robot2D_Dockspace");
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport -> Size);

        float startXSize = 400;

        auto treeNode = ImGui::DockBuilderAddNode();
        ImGui::DockBuilderSetNodeSize(treeNode, {startXSize, viewport -> Size.y});

        treeNode = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left,
                                               0.1f, nullptr,
                                               &dockspace_id);

        auto stats_id = ImGui::GetID("stats_id");
        auto tree_id = ImGui::DockBuilderSplitNode(treeNode, ImGuiDir_Up,
                                                   0.8f, nullptr,
                                                   &stats_id);
        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::ScenePanel].c_str(), tree_id);
        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::UtilsPanel].c_str(), stats_id);

        auto canvasNode = ImGui::DockBuilderAddNode();
        ImGui::DockBuilderSetNodeSize(treeNode, {viewport -> Size.x - startXSize, viewport -> Size.y});
        canvasNode = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left,
                                                 0.8f, nullptr,
                                                 &dockspace_id);

        auto assets_id = ImGui::GetID("assets_id");
        auto viewport_id = ImGui::DockBuilderSplitNode(canvasNode, ImGuiDir_Up,
                                                       0.8f, nullptr,
                                                       &assets_id);
        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::ViewportPanel].c_str(), viewport_id);
        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::GamePanel].c_str(), viewport_id);

        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::AssetsPanel].c_str(), assets_id);
        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::AnimationPanel].c_str(), assets_id);
        ImGui::DockBuilderDockWindow("Dear ImGui Demo", assets_id);

        auto inspector_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right,
                                                        0.1f, nullptr,
                                                        &dockspace_id);
        ImGui::DockBuilderDockWindow(windowIDS[UIPanelType::InspectorPanel].c_str(), inspector_id);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    void UIManager::blockEvents(bool flag) {}

    SceneEntity UIManager::getTreeItem(editor::UUID uuid) {
        return getPanel<ScenePanel>().getTreeItem(uuid);
    }

}