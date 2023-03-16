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

#pragma once

#include "IPanel.hpp"
#include <editor/Scene.hpp>
#include <editor/MessageDispather.hpp>
#include <editor/Messages.hpp>

namespace editor {

    struct ScenePanelConfiguration {
        const int rightMouseButton = 1;
    };

    class ScenePanel: public IPanel {
    public:
        ScenePanel(MessageDispatcher& messageDispatcher);
        ~ScenePanel()override = default;

        void setActiveScene(Scene::Ptr ptr) { m_scene = ptr; m_selectedEntity = {};}
        void render() override;

        robot2D::ecs::Entity getSelectedEntity() const { return m_selectedEntity; }
    private:
        void drawEntity(robot2D::ecs::Entity entity);
        void drawComponents(robot2D::ecs::Entity& entity);
        void onEntitySelection(const EntitySelection& entitySelection);

    private:
        MessageDispatcher& m_messageDispatcher;

        Scene::Ptr m_scene;
        robot2D::ecs::Entity m_selectedEntity;
        ScenePanelConfiguration m_configuration;
    };
}