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
#include <vector>
#include <memory>

#include <robot2D/Config.hpp>
#include <robot2D/Core/MessageBus.hpp>

#include "Bitmask.hpp"
#include "Defines.hpp"
#include "Entity.hpp"
#include "Component.hpp"

namespace robot2D::ecs {
    using EntityList = std::vector<Entity>;

    class Scene;
    class ROBOT2D_EXPORT_API System {
    public:
        using Ptr = std::shared_ptr<System>;
    public:
        System(robot2D::MessageBus& messageBus, UniqueType uniqueType);
        virtual ~System() = 0;

        bool fitsRequirements(Bitmask bitmask);
        bool addEntity(Entity entity);
        bool hasEntity(Entity entity);
        bool removeEntity(Entity entityId);
        Bitmask getSystemMask() const { return m_mask; }

        virtual void update(float dt);
        virtual void onMessage(const robot2D::Message& message);
    protected:
        friend class SystemManager;

        template<typename T>
        void addRequirement();

        void processRequirements(ComponentManager& componentManager);

        virtual void onEntityAdded(Entity entity);
        virtual void onEntityRemoved(Entity entity);

        void setScene(Scene*);
        Scene* getScene();

        const Scene* getScene() const { return m_scene; }

        bool cloneBase(Ptr clone, Scene* scene, const std::vector<Entity>& newEntities);
        virtual Ptr cloneSelf(Scene* scene, const std::vector<Entity>& newEntities) { return nullptr; }
    protected:
        UniqueType m_systemId;
        EntityList m_entities;
        robot2D::MessageBus& m_messageBus;
        std::vector<UniqueType> m_pendingTypes;
        Bitmask m_mask;
    private:
        Scene* m_scene;
    };

    template<typename T>
    void System::addRequirement() {
        UniqueType uniqueType(typeid(T));
        m_pendingTypes.emplace_back(uniqueType);
    }

}