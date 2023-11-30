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

#include <memory>
#include <cassert>
#include <functional>

#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Drawable.hpp>

#include "Components.hpp"
#include "physics/IPhysics2DAdapter.hpp"
#include "ScriptInteractor.hpp"
#include <editor/UIInteractor.hpp>
#include "DeletedEntitesRestoreInformation.hpp"

namespace editor {
    class Scene: public robot2D::Drawable {
    public:
        using Ptr = std::shared_ptr<Scene>;
    public:
        Scene(robot2D::MessageBus& messageBus);
        ~Scene() override = default;

        void createMainCamera();

        robot2D::ecs::EntityList& getEntities();
        const robot2D::ecs::EntityList& getEntities() const;

        void registerOnDeleteFinish(std::function<void()>&& callback);

        void update(float dt);
        void updateRuntime(float dt);

        void onRuntimeStart(ScriptInteractor::Ptr scriptInteractor);
        void onRuntimeStop();

        void setRuntimeCamera(bool flag);

        /// Serializer Api
        robot2D::ecs::Entity createEntity();
        robot2D::ecs::Entity createEmptyEntity();
        void addAssociatedEntity(robot2D::ecs::Entity entity);

        /// ScenePanel API
        void addEmptyEntity();
        void setBefore(robot2D::ecs::Entity source, robot2D::ecs::Entity target);
        void removeEntity(robot2D::ecs::Entity entity);
        void removeEntityChild(robot2D::ecs::Entity entity);
        DeletedEntitiesRestoreInformation removeEntities(std::vector<robot2D::ecs::Entity>& removingEntities);
        void restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation);


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

        [[maybe_unused]]
        robot2D::ecs::Entity
        duplicateEntity(robot2D::vec2f mousePos, robot2D::ecs::Entity entity);
        robot2D::ecs::Entity duplicateEmptyEntity(robot2D::ecs::Entity entity);

        const std::string& getPath() const {return m_path;}
        std::string& getPath()  {return m_path;}

        bool hasChanges() const { return m_hasChanges; }
    protected:
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;

    private:
        void initScene();
        void onPhysics2DRun();
        void onPhysics2DStop();

        void removeDuplicate(robot2D::ecs::Entity entity);
    private:
        friend class DuplicateCommand;

        robot2D::ecs::Scene m_scene;
        robot2D::ecs::EntityList m_sceneEntities;
        robot2D::ecs::EntityList m_deletePendingEntities;
        robot2D::ecs::EntityList m_deletePendingBuffer;

        robot2D::ecs::EntityList m_deletePendingPostEntities;
        robot2D::ecs::EntityList m_deletePendingPostBuffer;
        bool m_needWaitPostDeletion{false};

        robot2D::MessageBus& m_messageBus;
        std::string m_path;
        std::string m_associatedProjectPath;
        bool m_running = false;
        bool m_hasChanges{false};

        IPhysics2DAdapter::Ptr m_physicsAdapter{nullptr};

        enum class ReorderDeleteType {
            First, Last
        };

        using Iterator = std::vector<robot2D::ecs::Entity>::iterator;
        using InsertItem = std::tuple<Iterator, robot2D::ecs::Entity, ReorderDeleteType>;

        std::vector<InsertItem> m_insertItems;
        std::function<void()> m_onDeleteFinishCallback{nullptr};
    };
}