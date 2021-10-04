/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
ZombieArena - Zlib license.
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

#include <editor/Editor.hpp>
#include <editor/ComponentPanel.hpp>

namespace editor {

    Editor::Editor(): m_window({1280, 920}, "Editor", robot2D::WindowContext::Default) {}

    void Editor::setup() {
        auto& panel = m_panelManager.addPanel<ComponentPanel>();
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