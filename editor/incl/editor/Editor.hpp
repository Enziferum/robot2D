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

#pragma once

#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include "Wrapper.hpp"
#include "PanelManager.hpp"

namespace editor {
    class Editor {
    public:
        Editor();
        Editor(const Editor&)=delete;
        Editor(const Editor&&)=delete;
        Editor& operator=(const Editor&)=delete;
        Editor& operator=(const Editor&&)=delete;
        ~Editor() = default;

        void run();
    private:
        void setup();
        void handleEvents();
        void handleMessages();
        void update(float dt);
        void render();
    private:
        robot2D::RenderWindow m_window;
        robot2D::MessageBus m_messageBus;
        ImGui::Wrapper m_guiWrapper;
        PanelManager m_panelManager;
    };
}