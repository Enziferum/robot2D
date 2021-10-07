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

#include <memory>
#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Drawable.hpp>

namespace editor {
    class Scene: public robot2D::Drawable {
    public:
        using Ptr = std::shared_ptr<Scene>;
    public:
        Scene(robot2D::MessageBus& messageBus);
        ~Scene() = default;

        robot2D::ecs::EntityList& getEntities();
        const robot2D::ecs::EntityList& getEntities() const;

        void update(float dt);
        // Serializer Api
        robot2D::ecs::Entity createEntity();
        void removeEntity(robot2D::ecs::Entity entity);

        // ScenePanel API
        void addEmptyEntity();
        void setPath(const std::string& path) {m_path = path;}
        const std::string& getPath() const {return m_path;}
        std::string& getPath()  {return m_path;}
    protected:
        void draw(robot2D::RenderTarget &target, robot2D::RenderStates states) const override;

    private:
        void initScene();
    private:
        robot2D::ecs::Scene m_scene;
        robot2D::ecs::EntityList m_sceneEntities;
        robot2D::MessageBus& m_messageBus;
        std::string m_path;
    };
}