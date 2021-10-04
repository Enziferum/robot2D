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
#include <cassert>
#include <robot2D/Graphics/Drawable.hpp>
#include <robot2D/Core/MessageBus.hpp>

#include "EntityManager.hpp"
#include "SystemManager.hpp"

namespace robot2D::ecs {
    // Ecs Manager as well, entites adding to Scene
    class Scene: public robot2D::Drawable {
    public:
        Scene(robot2D::MessageBus& messageBus);
        Scene(const Scene&)=delete;
        Scene(const Scene&&)=delete;
        Scene& operator=(const Scene&)=delete;
        Scene& operator=(const Scene&&)=delete;
        ~Scene() = default;

        Entity createEntity();
        void removeEntity(Entity entity);

        template<class T, typename ...Args>
        void addSystem(Args&& ...args);

        void update(float dt);
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates) const override;
    private:
        robot2D::MessageBus& m_messageBus;
        ComponentManager m_componentManager;
        EntityManager m_entityManager;
        SystemManager m_systemManager;

        std::vector<Entity> m_addPending;
        std::vector<Entity> m_deletePending;
        std::vector<robot2D::Drawable*> m_drawables;
    };

    template<typename T, typename ...Args>
    void Scene::addSystem(Args&& ...args) {
        static_assert(std::is_base_of<System, T>::value, "Adding T must be subclass of ecs::System");

        auto& system = m_systemManager.addSystem<T>(std::forward<Args>(args)...);
        if(std::is_base_of<robot2D::Drawable, T>::value) {
            auto* drawable = dynamic_cast<robot2D::Drawable*>(&system);
            m_drawables.emplace_back(drawable);
        }
    }

}