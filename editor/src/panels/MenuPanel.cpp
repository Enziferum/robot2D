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



        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
                fileMenu();

            if (ImGui::BeginMenu("Edit"))
                editMenu();

            if (ImGui::BeginMenu("Project"))
                projectMenu();

            if (ImGui::BeginMenu("Plugins"))
                pluginsMenu();

            if (ImGui::BeginMenu("Developer")) {
                ImGui::MenuItem("Show Info", nullptr, &openDeveloperMenu);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
                helpMenu();

            ImGui::EndMenuBar();
        }
    }

    void MenuPanel::fileMenu() {
        if(ImGui::MenuItem("New", "Ctrl+N")) {
            std::string path;
            if(!FiledialogAdapter::get() -> selectFolder(path, "Create Robot2D Project")) {
                ImGui::EndMenu();
                return;
            }

            ProjectDescription description;
            description.name = "Project";
            description.path = path;

            auto msg = m_messageBus.postMessage<ProjectMessage>(MessageID::CreateProject);
            msg -> description = std::move(description);

            ImGui::EndMenu();
            return;
        }
        ImGui::Separator();

        if(ImGui::MenuItem("Open", "Ctrl+O")) {
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
                ImGui::EndMenu();
                return;
            }
            else {
                ImGui::EndMenu();
                return;
            }
        }

        if(ImGui::MenuItem("Save", "Ctrl+S")) {
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

        if(ImGui::MenuItem("Save", "Ctrl+Shift+S")) {
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

        ImGui::EndMenu();
    }

    void MenuPanel::editMenu() {
        bool emptyStack = m_interactor -> getCommandStack().empty();

        if(ImGui::MenuItem("Undo", "Ctrl+Z", false, !emptyStack)) {
            m_messageBus.postMessage<UndoRedoMessage>(MessageID::Undo);
        }

        if(ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, false)) {
            m_messageBus.postMessage<UndoRedoMessage>(MessageID::Redo);
        }

        ImGui::EndMenu();
    }

    void MenuPanel::projectMenu() {
        if(ImGui::MenuItem("Generate Project", "Ctrl+G")) {
            std::string genPath;
            if(!FiledialogAdapter::get() -> selectFolder(genPath,
                                                         "Generate Robot2D Scripts's project",
                                                         "")) {
                ImGui::EndMenu();
                return;
            }

            auto* msg =
                    m_messageBus.postMessage<GenerateProjectMessage>(MessageID::GenerateProject);
            msg -> genPath = genPath;
        }


        if(ImGui::MenuItem("Export Project")) {
            auto* manager = PopupManager::getManager();
            manager -> beginPopup(this);
            m_popupType = PopupType::Export;
        }
        ImGui::EndMenu();
    }

    void MenuPanel::pluginsMenu() {

        if(ImGui::MenuItem("LoadPlugin")) {
            auto filedialog = FiledialogAdapter::get();
            std::string openFilePath{};
            if(!filedialog -> openFile(openFilePath, "Plugin")) {
                RB_EDITOR_ERROR("Can't open file by path {0}", openFilePath);
                /// TODO(a.raag): use Defer to close EndMenu()
                return;
            }
        }

        ImGui::EndMenu();
    }

    void MenuPanel::developerMenu() {
        if (!ImGui::Begin("Debug Info"))
        {
            ImGui::End();
            return;
        }
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
        ImGui::End();
    }

    void MenuPanel::helpMenu() {
        ImGui::EndMenu();
    }

    void MenuPanel::showExportProjectModal() {
        ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("ExportProject");

        if(ImGui::BeginPopupModal("ExportProject", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Export Project");

            ImGui::RadioButton("Fullscreen", m_exportOptions.isFullscreen);

            std::vector<std::string> scenesNames = {"Main.scene", "Menu.scene"};
            const char* currentSceneName = scenesNames[0].c_str();
            if (ImGui::BeginCombo("Start Scene", currentSceneName))
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

                ImGui::EndCombo();
            }

            if(ImGui::Button("Close")) {
                m_popupType = PopupType::None;
                PopupManager::getManager() -> endPopup();
                m_exportOptions = {};
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::Button("Export")) {
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


            ImGui::EndPopup();
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
