//
// Created by Necromant on 03.10.2021.
//

#include <editor/Editor.hpp>

namespace editor {

    Editor::Editor(): m_window({1280, 920}, "Editor", robot2D::WindowContext::Default) {}

    void Editor::setup() {

        m_guiWrapper.init(m_window);
    }

    void Editor::run() {
        setup();

        while(m_window.isOpen()) {
            handleEvents();
            handleMessages();
            update(0.F);
            m_guiWrapper.update(0.F);
            render();
        }
    }

    void Editor::handleEvents() {
        robot2D::Event event{};
        while (m_window.pollEvents(event)) {
            m_guiWrapper.handleEvents(event);
        }
    }

    void Editor::handleMessages() {
        robot2D::Message message{};
        while (m_messageBus.pollMessages(message)) {

        }
    }

    void Editor::update(float dt) {
        m_panelManager.update(dt);
    }

    void Editor::render() {
        m_window.clear();
        m_window.beforeRender();

        //

        m_window.afterRender();
        m_window.flushRender();

        m_panelManager.render();
        m_guiWrapper.render();
        m_window.display();
    }


}