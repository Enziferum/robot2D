/*********************************************************************
(c) Alex Raag 2023
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

#include <imgui/imgui.h>
#include <tfd/tinyfiledialogs.h>
#include <filesystem>

#include <robot2D/imgui/Gui.hpp>
#include <editor/panels/MenuPanel.hpp>
#include <editor/Messages.hpp>
#include <editor/PopupManager.hpp>

namespace editor {

    namespace fs = std::filesystem;

    MenuPanel::MenuPanel(robot2D::MessageBus& messageBus):
        IPanel(UniqueType(typeid(MenuPanel))),
        m_messageBus{messageBus} {}

    void MenuPanel::render() {
        ImGui::ShowDemoWindow();
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
                fileMenu();
            if (ImGui::BeginMenu("Edit"))
                editMenu();
            if (ImGui::BeginMenu("Project"))
                projectMenu();

            /* TODO(a.raag) other task for help
            if (ImGui::BeginMenu("Help"))
                helpMenu();
            */
            ImGui::EndMenuBar();
        }
    }

    void MenuPanel::fileMenu() {
        if(ImGui::MenuItem("New", "Ctrl+N")) {
            const char* path = tinyfd_selectFolderDialog("Create Robot2D Project", nullptr);
            // close, either
            if(path == nullptr) {
                return;
            }
            std::string creationPath(path);
            ProjectDescription description;
            description.name = "Project";
            description.path = creationPath;

            auto msg = m_messageBus.postMessage<ProjectMessage>(MessageID::CreateProject);
            msg -> description = std::move(description);
        }
        ImGui::Separator();

        if(ImGui::MenuItem("Open", "Ctrl+O")) {
            std::string openPath = "assets/scenes/demoScene.robot2D";
            const char* patterns[1] = {"*.robot2D"};
            const char* path = tinyfd_openFileDialog("Load Project", nullptr,
                                                     1,
                                                     patterns,
                                                     "Project",
                                                     0);
            if (path != nullptr) {
                fs::path openPath{std::string(path)};
                auto filePath = openPath.filename();
                openPath.remove_filename();
                auto str = filePath.string();
                auto found = str.find('.');
                std::string name = str.substr(0, found);

                auto msg = m_messageBus.postMessage<ProjectMessage>(MessageID::LoadProject);
                msg -> description.path = openPath.string();
                msg -> description.name = name;
            }
        }

        if(ImGui::MenuItem("Save", "Ctrl+S")) {
            std::string savePath = "assets/scenes/demoScene.robot2D";
            const char* patterns[1] = {"*.robot2D"};
            const char* path = tinyfd_saveFileDialog("Save Scene", nullptr,
                                                     1, patterns,
                                                     "Scene");
            if(path != nullptr) {
                savePath = std::string(path);
                auto msg =
                        m_messageBus.postMessage<MenuProjectMessage>(MessageID::SaveProject);
                msg -> path = std::move(savePath);
            }
        }

        if(ImGui::MenuItem("Save", "Ctrl+Shift+S")) {
            std::string savePath = "assets/scenes/demoScene.robot2D";
            const char* patterns[1] = {"*.scene"};
            const char* path = tinyfd_saveFileDialog("Save Scene", nullptr,
                                                     1, patterns,
                                                     "Scene");
            if(path != nullptr) {
                savePath = std::string(path);
                auto msg =
                        m_messageBus.postMessage<MenuProjectMessage>(MessageID::SaveScene);
                msg -> path = std::move(savePath);
            }
        }

        ImGui::EndMenu();
    }

    void MenuPanel::editMenu() {
        if(ImGui::MenuItem("Undo", "Ctrl+Z")) {
            m_messageBus.postMessage<UndoRedoMessage>(MessageID::Undo);
        }

        if(ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, false)) {
            m_messageBus.postMessage<UndoRedoMessage>(MessageID::Redo);
        }

        ImGui::EndMenu();
    }

    void MenuPanel::projectMenu() {
        if(ImGui::MenuItem("Generate Project", "Ctrl+G"))
            m_messageBus.postMessage<GenerateProjectMessage>(MessageID::GenerateProject);

        if(ImGui::MenuItem("Export Project")) {
            auto* manager = PopupManager::getManager();
            manager -> beginPopup(this);
            m_popupType = PopupType::Export;
        }
        ImGui::EndMenu();
    }

    void MenuPanel::helpMenu() {
        ImGui::EndMenu();
    }


    void MenuPanel::showExportProjectModal() {
        ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("Export Project!");
        if(ImGui::BeginPopupModal("Export Project!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Modal Text!");

            if(ImGui::Button("Ok")) {
                m_popupType = PopupType::None;
                PopupManager::getManager() -> endPopup();
                ImGui::CloseCurrentPopup();
            }



            // 1. Choose StartScene
            // 2. Choose TargetResolution
            // 3. Choose Folder


            // 1. Main Scene
            //
            //auto* msg = m_messageBus.postMessage<ExportProjectMessage>(MessageID::ExportProject);
            ImGui::EndPopup();
        }
    }

    void MenuPanel::onRender() {
        switch(m_popupType) {
            case PopupType::None:
                break;
            case PopupType::NewProject:
                break;
            case PopupType::OpenProject:
                break;
            case PopupType::Export:
                showExportProjectModal();
                break;
        }
    }

}
