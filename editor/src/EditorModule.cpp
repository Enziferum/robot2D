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

#include <editor/EditorModule.hpp>

namespace editor {

    IEditorOpener::~IEditorOpener() = default;
    IEditorModule::~IEditorModule() = default;

    EditorModule::EditorModule(robot2D::MessageBus& messageBus,
                               MessageDispatcher& messageDispatcher, robot2D::Gui& gui)
    : m_editor{messageBus, gui},
      m_router{&m_editor},
      m_presenter{&m_editor},
      m_interactor{new EditorLogic(messageBus, messageDispatcher, m_presenter, m_router)},
      m_messageDispatcher{messageDispatcher}
    {
        m_messageDispatcher.onMessage<ToolbarMessage>(MessageID::ToolbarPressed,
                                                      BIND_CLASS_FN(onSwitchRuntimeState));
    }

    void EditorModule::setup(robot2D::RenderWindow* window, ScriptingEngineService::Ptr scriptingEngine) {
        m_editor.setup(window, m_interactor.get());

        EditorInteractor::WeakPtr weakEditorInteractor = m_interactor;
        m_scriptInteractor = std::make_shared<ScriptInteractor>(weakEditorInteractor, scriptingEngine);
        m_interactor -> setup(m_scriptInteractor);
    }

    void EditorModule::handleEvents(const robot2D::Event& event) {
        m_editor.handleEvents(event);
    }

    void EditorModule::handleMessages(const robot2D::Message& message) {
        m_editor.handleMessages(message);
        ////
    }

    void EditorModule::update(float dt) {
        m_editor.update(dt);
    }

    void EditorModule::render() {
        m_editor.render();
    }


    void EditorModule::createProject(Project::Ptr project) {
        m_interactor -> createProject(project);
    }

    void EditorModule::loadProject(Project::Ptr project) {
        m_interactor -> loadProject(project);
    }

    void EditorModule::destroy() {
        m_interactor -> destroy();
    }

    void EditorModule::closeCurrentProject(std::function<void()>&& resultCallback) {
        m_interactor -> closeCurrentProject(std::move(resultCallback));
    }

    void EditorModule::onSwitchRuntimeState(const ToolbarMessage& toolbarMessage) {
        m_interactor -> switchRuntimeState(toolbarMessage.type);
    }

} // namespace editor

