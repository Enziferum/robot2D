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

#pragma once
#include <robot2D/Core/MessageBus.hpp>
#include "IPanel.hpp"
#include <editor/PopupManager.hpp>

namespace editor {
    class MenuPanel: public IPanel, public PopupDelegate {
    public:
        MenuPanel(robot2D::MessageBus& messageBus);
        MenuPanel(const MenuPanel& other) = delete;
        MenuPanel& operator=(const MenuPanel& other) = delete;
        MenuPanel(MenuPanel&& other) = delete;
        MenuPanel& operator=(MenuPanel&& other) = delete;
        ~MenuPanel() override = default;


        void render() override;
    private:
        void fileMenu();
        void editMenu();
        void projectMenu();
        void showExportProjectModal();
        void helpMenu();

        void onRender() override;
    private:
        robot2D::MessageBus& m_messageBus;
        enum class PopupType {
            None,
            OpenProject,
            NewProject,
            Export
        } m_popupType = PopupType::None;
    };
}
