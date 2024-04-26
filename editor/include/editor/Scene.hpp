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
#include <list>

#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Drawable.hpp>

#include <editor/UIInteractor.hpp>
#include "Components.hpp"
#include "physics/IPhysics2DAdapter.hpp"
#include "ScriptInteractor.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "EditorCamera.hpp"
#include "SceneEntity.hpp"
#include "SceneGraph.hpp"

namespace editor {

    class Scene: public robot2D::Drawable {
    public:
        using Ptr = std::shared_ptr<Scene>;
        using EntityList = std::list<robot2D::ecs::Entity>;
    public:
        Scene(robot2D::MessageBus& messageBus);
        Scene(const Scene& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(Scene&& other) = delete;
        ~Scene() override = default;

        void createMainCamera();

        std::list<SceneEntity>& getEntities();
        const EntityList& getEntities() const;

        void registerOnDeleteFinish(std::function<void()>&& callback);

        void handleEventsRuntime(const robot2D::Event& event);
        void update(float dt);
        void updateRuntime(float dt);

        void onRuntimeStart(ScriptInteractor::Ptr scriptInteractor);
        void onRuntimeStop();

        void setRuntimeCamera(bool flag);

        //////////////////////// Serializer Api ////////////////////////
        SceneEntity createEntity();
        void addAssociatedEntity(SceneEntity&& entity);
        SceneEntity createEmptyEntity();

        /// (a.raag): tmp method for refactor usages
        void convertEntities();
        //////////////////////// Serializer Api ////////////////////////

        /////////////////////////// ScenePanel API ///////////////////////////
        void addEmptyEntity();
        SceneEntity addEmptyButton();
        /////////////////////////// ScenePanel API ///////////////////////////


        void setBefore(SceneEntity source, SceneEntity target);
        void removeEntity(SceneEntity entity);
        void removeEntityChild(SceneEntity entity);
        DeletedEntitiesRestoreInformation removeEntities(std::vector<SceneEntity>& removingEntities);
        void restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation);


        robot2D::ecs::Entity getByUUID(UUID uuid);
        SceneEntity getEntity(UUID uuid) const;


        bool isRunning() const { return m_running; }

        void setPath(const std::string& path) {m_path = path;}
        void setAssociatedProjectPath(const std::string& path) { m_associatedProjectPath = path;}

        const std::string& getAssociatedProjectPath() const {
            return m_associatedProjectPath;
        }

        [[maybe_unused]]
        SceneEntity
        duplicateEntity(robot2D::vec2f mousePos, SceneEntity entity);
        SceneEntity duplicateEmptyEntity(SceneEntity entity);

        [[maybe_unused]]
        SceneEntity duplicateRuntime(SceneEntity entity, robot2D::vec2f position);

        const std::string& getPath() const {return m_path;}
        std::string& getPath()  {return m_path;}

        void setEditorCamera(EditorCamera::Ptr editorCamera);

        bool hasChanges() const { return m_hasChanges; }
    protected:
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;

    private:
        struct RemoveEntityInfo {
            robot2D::ecs::Entity entity;
            bool isDeleted{false};
        };

        robot2D::ecs::Entity getByUUID(robot2D::ecs::Entity, UUID uuid);
        void initScene();
        void onPhysics2DRun();
        void onPhysics2DStop();

        void removeDuplicate(SceneEntity entity);
        void removeChildEntities(DeletedEntitiesRestoreInformation& information,
                                 std::vector<RemoveEntityInfo>& removingEntities,
                                 robot2D::ecs::Entity parent, bool isParentDel = false);

        void duplicateEntityChild(SceneEntity parent, SceneEntity dupEntity);

        void registerUICallback(SceneEntity uiEntity);
    private:
        friend class EditorLogic;
        friend class DuplicateCommand;

        robot2D::ecs::Scene m_scene;
        robot2D::ecs::Scene m_CloneScene;
        IEditorCamera::Ptr m_editorCamera{nullptr};

        EntityList m_sceneEntities;
        robot2D::ecs::EntityList m_deletePendingEntities;
        robot2D::ecs::EntityList m_deletePendingBuffer;


        robot2D::MessageBus& m_messageBus;
        std::string m_path;
        std::string m_associatedProjectPath;
        bool m_running = false;
        bool m_hasChanges{false};

        IPhysics2DAdapter::Ptr m_physicsAdapter{nullptr};

        enum class ReorderDeleteType {
            First, Last
        };

        using Iterator = EntityList::iterator;
        using InsertItem = std::tuple<Iterator, robot2D::ecs::Entity, ReorderDeleteType>;
        using SetItem = std::tuple<Iterator, robot2D::ecs::Entity, bool, robot2D::ecs::Entity>;

        std::vector<InsertItem> m_insertItems;
        std::vector<SetItem> m_setItems;
        std::function<void()> m_onDeleteFinishCallback{nullptr};


        SceneGraph m_sceneGraph;
    };
}