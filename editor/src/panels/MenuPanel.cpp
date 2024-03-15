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
#include <filesystem>

#include <robot2D/imgui/Api.hpp>
#include <robot2D/Util/Logger.hpp>

#include <editor/panels/MenuPanel.hpp>
#include <editor/Messages.hpp>
#include <editor/PopupManager.hpp>
#include <editor/FiledialogAdapter.hpp>

namespace editor {

    namespace fs = std::filesystem;
    namespace  {
        static bool openDeveloperMenu = false;
        static bool openPluginMenu = false;
    }

    MenuPanel::MenuPanel(robot2D::MessageBus& messageBus, UIInteractor::Ptr interactor):
        IPanel(UniqueType(typeid(MenuPanel))),
        m_messageBus{messageBus},
        m_interactor{interactor}{}

    void MenuPanel::render() {
        if(openDeveloperMenu)
            developerMenu();

        imgui_MenuBar {
            imgui_Menu("File")
                fileMenu();

            imgui_Menu("Edit")
                editMenu();

            imgui_Menu("Project")
                projectMenu();

            imgui_Menu("Plugins")
                pluginsMenu();

            imgui_Menu("Developer")
                ImGui::MenuItem("Show Info", nullptr, &openDeveloperMenu);

            imgui_Menu("Help")
                helpMenu();
        }

    }

    void MenuPanel::fileMenu() {
        imgui_MenuItem("New", "Ctrl + N") {
            std::string path;
            if(!FiledialogAdapter::get() -> selectFolder(path, "Create Robot2D Project")) {
                return;
            }

            ProjectDescription description;
            description.name = "Project";
            description.path = path;

            auto msg = m_messageBus.postMessage<ProjectMessage>(MessageID::CreateProject);
            msg -> description = std::move(description);
            return;
        }
        ImGui::Separator();

        imgui_MenuItem("Open", "Ctrl+O") {
            std::vector<std::string> openPatterns = {"*.robot2D"};
            std::string outputPath;

            if (FiledialogAdapter::get() -> openFile(outputPath,
                                                     "Open robot2D project",
                                                     "",
                                                     openPatterns,
                                                     "Robot2D Project")) {
                fs::path openPath{std::string(outputPath)};
                auto filePath = openPath.filename();
                openPath.remove_filename();
                auto str = filePath.string();
                auto found = str.find('.');
                std::string name = str.substr(0, found);

                auto msg = m_messageBus.postMessage<ProjectMessage>(MessageID::LoadProject);
                msg -> description.path = openPath.string();
                msg -> description.name = name;
                return;
            }
        }

        imgui_MenuItem("Save", "Ctrl+S") {
            std::string savePath;
            std::vector<std::string> saveFilePatterns = {"*.robot2D"};

            // TODO(a.raag): get current project path
            if(FiledialogAdapter::get() -> saveFile(savePath,
                                                    "Save Robot2D Project",
                                                    "",
                                                    saveFilePatterns,
                                                    "Scene")) {
                auto msg =
                        m_messageBus.postMessage<MenuProjectMessage>(MessageID::SaveProject);
                msg -> path = std::move(savePath);
            }
        }

        imgui_MenuItem("Save", "Ctrl+Shift+S") {
            std::string savePath;
            std::vector<std::string> saveFilePatterns = {"*.scene"};

            // TODO(a.raag): get current project path
            std::string defaultSavePath = "assets/scenes";

            if(FiledialogAdapter::get() -> saveFile(savePath,
                                                    "Save Robot2D Scene",
                                                     defaultSavePath,
                                                     saveFilePatterns,
                                                     "Scene")){
                auto msg =
                        m_messageBus.postMessage<MenuProjectMessage>(MessageID::SaveScene);
                msg -> path = std::move(savePath);
            }
        }
    }

    void MenuPanel::editMenu() {
        bool emptyStack = m_interactor -> getCommandStack().empty();

        imgui_MenuItem("Undo", "Ctrl+Z", false, !emptyStack) {
            m_messageBus.postMessage<UndoRedoMessage>(MessageID::Undo);
        }

        imgui_MenuItem("Redo", "Ctrl+Shift+Z", false, false) {
            m_messageBus.postMessage<UndoRedoMessage>(MessageID::Redo);
        }
    }

    void MenuPanel::projectMenu() {
        imgui_MenuItem("Generate Project", "Ctrl+G") {
            std::string genPath;
            if(!FiledialogAdapter::get() -> selectFolder(genPath,
                                                         "Generate Robot2D Scripts's project",
                                                         "")) {
                return;
            }

            auto* msg =
                    m_messageBus.postMessage<GenerateProjectMessage>(MessageID::GenerateProject);
            msg -> genPath = genPath;
        }


        imgui_MenuItem("Export Project") {
            auto* manager = PopupManager::getManager();
            manager -> beginPopup(this);
            m_popupType = PopupType::Export;
        }
    }

    void MenuPanel::pluginsMenu() {
        imgui_MenuItem("LoadPlugin") {
            auto filedialog = FiledialogAdapter::get();
            std::string openFilePath{};
            if(!filedialog -> openFile(openFilePath, "Plugin")) {
                RB_EDITOR_ERROR("Can't open file by path {0}", openFilePath);
                /// TODO(a.raag): use Defer to close EndMenu()
                return;
            }
        }
    }

    void MenuPanel::developerMenu() {
        imgui_Window("Debug Info") {
            const auto& commandStack = m_interactor -> getCommandStack();

            if (ImGui::BeginListBox("CommandsPool"))
            {
                int cntr = 1;
                for (int n = commandStack.size() - 1; n >= 0; --n)
                {
                    ImGui::Text("%i. %s", cntr, commandStack[n].getName());
                    ++cntr;
                }
                ImGui::EndListBox();
            }
        }
    }

    void MenuPanel::helpMenu() {}

    void MenuPanel::showExportProjectModal() {
        ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("ExportProject");

        imgui_PopupModal("ExportProject", nullptr, ImGuiWindowFlags_AlwaysAutoResize) {
            ImGui::Text("Export Project");

            ImGui::RadioButton("Fullscreen", m_exportOptions.isFullscreen);

            std::vector<std::string> scenesNames = {"Main.scene", "Menu.scene"};
            const char* currentSceneName = scenesNames[0].c_str();
            /// TODO(a.raag): add some macro to remove shadowing
            imgui_Combo("Start Scene", currentSceneName)
            {
                for (int i = 0; i < scenesNames.size(); i++)
                {
                    bool isSelected = currentSceneName == scenesNames[i];
                    if (ImGui::Selectable(scenesNames[i].c_str(), isSelected))
                    {
                        currentSceneName = scenesNames[i].c_str();
                        m_exportOptions.startScene = currentSceneName;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }

            imgui_Button("Close") {
                m_popupType = PopupType::None;
                PopupManager::getManager() -> endPopup();
                m_exportOptions = {};
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            imgui_Button("Export") {
                m_popupType = PopupType::None;
                std::string exportPath;

                if(FiledialogAdapter::get() -> selectFolder(exportPath, "Export Robot2D Project")) {
                    m_exportOptions.outputFolder = exportPath;
                    m_interactor -> exportProject(m_exportOptions);
                }

                m_exportOptions = {};
                PopupManager::getManager() -> endPopup();
                ImGui::CloseCurrentPopup();
            }
        }
    }

    void MenuPanel::onPopupRender() {
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
