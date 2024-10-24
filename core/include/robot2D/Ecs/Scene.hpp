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

#include <cassert>
#include <robot2D/Graphics/Drawable.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Config.hpp>

#include "EntityManager.hpp"
#include "SystemManager.hpp"

namespace robot2D::ecs {

    /// TODO(a.raag): some SFINAE Rules for this class and add some functionality
    template<typename Container>
    class DoubleBuffer {
    public:
        using T = typename Container::value_type;
    public:
        DoubleBuffer() = default;
        DoubleBuffer(const DoubleBuffer& other) = delete;
        DoubleBuffer& operator=(const DoubleBuffer& other) = delete;
        DoubleBuffer(DoubleBuffer&& other) = delete;
        DoubleBuffer& operator=(DoubleBuffer&& other) = delete;
        ~DoubleBuffer() = default;

        void push_back(const T& value) { m_containerBuffer.push_back(value); }
        void update() { m_container.swap(m_containerBuffer); }
        void clear() { m_container.clear(); }
        const Container& getData() const { return m_container; }
    private:
        Container m_container;
        Container m_containerBuffer;
    };


    /// \brief Ecs Manager as well, entities adding to Scene
    class ROBOT2D_EXPORT_API Scene: public robot2D::Drawable {
    public:
        Scene(robot2D::MessageBus& messageBus, bool useSystems = true);
        Scene(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene& operator=(Scene&&) = delete;
        ~Scene() = default;

        bool cloneSelf(Scene& clone, std::vector<Entity>& newArray, bool cloneSystems = false);

        /// \brief usefull for clone if want reuse clone
        bool clearSelf();

        Entity createEntity();
        /// \brief Allocate Entity, but not add it systems. Useful for buffer
        Entity createEmptyEntity();

        void addEntity(robot2D::ecs::Entity);
        Entity duplicateEntity(robot2D::ecs::Entity entity);


        void removeEntity(Entity entity);
        bool restoreEntity(Entity entity);

        template<class T, typename ...Args>
        void addSystem(Args&& ...args);

        template<typename T>
        bool hasSystem() const;

        template<typename T>
        T* getSystem();

        template<typename T>
        const T* getSystem() const;

        void handleMessages(const Message& message);

        void update(float dt);

        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
    private:
        friend class SystemManager;
        robot2D::MessageBus& m_messageBus;
        ComponentManager m_componentManager;
        EntityManager m_entityManager;
        SystemManager m_systemManager;

        using EntityContainer = std::vector<Entity>;
        ///DoubleBuffer<EntityContainer> m_addBuffer;
        EntityContainer m_addBuffer;
        DoubleBuffer<EntityContainer> m_deleteBuffer;

        std::vector<robot2D::Drawable*> m_drawables;
        bool m_useSystems;
    };

    template<typename T, typename ...Args>
    void Scene::addSystem(Args&& ...args) {
        static_assert(std::is_base_of_v<System, T>, "T must be subclass of ecs::System");

        auto& system = m_systemManager.addSystem<T>(std::forward<Args>(args)...);
        if(std::is_base_of_v<robot2D::Drawable, T>) {
            auto* drawable = dynamic_cast<robot2D::Drawable*>(&system);
            m_drawables.emplace_back(drawable);
        }
    }

    template<typename T>
    T* Scene::getSystem() {
        return m_systemManager.getSystem<T>();
    }

    template<typename T>
    const T* Scene::getSystem() const {
        return m_systemManager.getSystem<T>();
    }

    template<typename T>
    bool Scene::hasSystem() const {
        return m_systemManager.hasSystem<T>();
    }

}