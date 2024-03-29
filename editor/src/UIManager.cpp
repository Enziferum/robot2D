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

#include <editor/UIManager.hpp>
#include <editor/panels/ScenePanel.hpp>

namespace editor {

    IUIManager::~IUIManager() = default;

    UIManager::UIManager(robot2D::Gui& gui):
        m_gui{gui}, m_panels() {}

    void UIManager::update(float dt) {
        for(auto& panel: m_panels)
            panel -> update(dt);
    }

    void UIManager::render() {
        for(auto& panel: m_panels)
            panel -> render();
    }

    void UIManager::dockingCanvas() {

    }

    void UIManager::blockEvents(bool flag) {
       // m_gui.blockEvents(flag);
    }

    robot2D::ecs::Entity UIManager::getSelectedEntity(int PixelData) {
        return getPanel<ScenePanel>().getSelectedEntity(PixelData);
    }


    robot2D::ecs::Entity UIManager::getSelectedEntity() {
        return getPanel<ScenePanel>().getSelectedEntity();
    }

    robot2D::ecs::Entity UIManager::getTreeItem(editor::UUID uuid) {
        return getPanel<ScenePanel>().getTreeItem(uuid);
    }
}