/*********************************************************************
(c) Alex Raag 2021
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

#include <robot2D/Extra/Api.hpp>
#include <editor/panels/MenuPanel.hpp>
#include <editor/Messages.hpp>

namespace editor {

    MenuPanel::MenuPanel(robot2D::MessageBus& messageBus):
        IPanel(UniqueType(typeid(MenuPanel))),
        m_messageBus{messageBus} {}

    void MenuPanel::render() {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
              fileMenu();
            if (ImGui::BeginMenu("Edit"))
                editMenu();
            if (ImGui::BeginMenu("Project"))
                projectMenu();
            if (ImGui::BeginMenu("Help"))
                helpMenu();
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
                openPath = std::string(path);
                auto msg =
                        m_messageBus.postMessage<MenuProjectMessage>(MessageID::OpenProject);
                msg -> path = std::move(openPath);
            }
        }

        if(ImGui::MenuItem("Open", "Ctrl+Shift+O")) {
            std::string openPath = "assets/scenes/demoScene.robot2D";
            const char* patterns[1] = {"*.scene"};
            const char* path = tinyfd_openFileDialog("Load Scene", nullptr,
                                                     1,
                                                     patterns,
                                                     "Scene",
                                                     0);
            if (path != nullptr) {
                openPath = std::string(path);
                auto msg =
                        m_messageBus.postMessage<MenuProjectMessage>(MessageID::OpenScene);
                msg -> path = std::move(openPath);
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
        }
        if(ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, false)) {
        }
        ImGui::EndMenu();
    }

    void MenuPanel::projectMenu() {
        if(ImGui::MenuItem("Generate Project", "Ctrl+G")) {
        }
        ImGui::EndMenu();
    }

    void MenuPanel::helpMenu() {
        ImGui::EndMenu();
    }
}
