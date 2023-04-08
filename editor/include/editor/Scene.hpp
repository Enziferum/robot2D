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
#include <cassert>

#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Drawable.hpp>

#include "Components.hpp"
#include "physics/IPhysics2DAdapter.hpp"

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
        void updateRuntime(float dt);

        void onRuntimeStart();
        void onRuntimeStop();


        // Serializer Api
        robot2D::ecs::Entity createEntity();
        void removeEntity(robot2D::ecs::Entity entity);
        void removeEntityChild(robot2D::ecs::Entity entity);

        // ScenePanel API
        void addEmptyEntity();

        void addAssociatedEntity(robot2D::ecs::Entity entity);

        robot2D::ecs::Entity getByIndex(int index) {
            return m_sceneEntities[index];
        }

        robot2D::ecs::Entity getByUUID(UUID uuid) {
            auto found = std::find_if(m_sceneEntities.begin(), m_sceneEntities.end(), [&uuid](robot2D::ecs::Entity entity) {
                auto& ts = entity.getComponent<TransformComponent>();
                if(ts.hasChildren()) {
                    for(auto& child: ts.getChildren()) {
                        if(child.getComponent<IDComponent>().ID == uuid)
                            return true;
                    }
                }
                return entity.getComponent<IDComponent>().ID == uuid;
            });
            assert(found != m_sceneEntities.end() && "Must be validUUID");
            return *found;
        }

        bool isRunning() const { return m_running; }

        void setPath(const std::string& path) {m_path = path;}
        void setAssociatedProjectPath(const std::string& path) { m_associatedProjectPath = path;}

        const std::string& getAssociatedProjectPath() const {
            return m_associatedProjectPath;
        }

        const std::string& getPath() const {return m_path;}
        std::string& getPath()  {return m_path;}
    protected:
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;

    private:
        void initScene();
        void onPhysics2DRun();
        void onPhysics2DStop();
    private:
        robot2D::ecs::Scene m_scene;
        robot2D::ecs::EntityList m_sceneEntities;
        robot2D::MessageBus& m_messageBus;
        std::string m_path;
        std::string m_associatedProjectPath;
        bool m_running = false;

        IPhysics2DAdapter::Ptr m_physicsAdapter{nullptr};
    };
}