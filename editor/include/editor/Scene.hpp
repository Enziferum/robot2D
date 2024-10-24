/*********************************************************************
(c) Alex Raag 2024
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

    class Scene : public robot2D::Drawable {
    public:
        using Ptr = std::shared_ptr<Scene>;
        using TraverseFunction = std::function<void(SceneEntity&)>;

    public:
        explicit Scene(robot2D::MessageBus& messageBus);
        Scene(const Scene& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(Scene&& other) = delete;
        ~Scene() override = default;

        void createMainCamera();

        std::list<SceneEntity>& getEntities();
        const std::list<SceneEntity>& getEntities() const;

        void handleEventsRuntime(const robot2D::Event& event);
        void update(float dt);
        void updateRuntime(float dt, IScriptInteractorFrom::Ptr scriptInteractor);

        void onRuntimeStart(IScriptInteractorFrom::Ptr scriptInteractor);
        void onRuntimeStop(IScriptInteractorFrom::Ptr scriptInteractor);

        void setRuntimeCamera(bool flag);

        //////////////////////// Serializer Api ////////////////////////
        SceneEntity createEntity();
        void addAssociatedEntity(SceneEntity&& entity);
        SceneEntity createEmptyEntity();

        //////////////////////// Serializer Api ////////////////////////

        /////////////////////////// ScenePanel API ///////////////////////////
        SceneEntity addEmptyEntity();
        SceneEntity addEmptyButton();
        /////////////////////////// ScenePanel API ///////////////////////////


        bool setBefore(SceneEntity source, SceneEntity target);
        void removeEntityChild(SceneEntity entity);
        DeletedEntitiesRestoreInformation removeEntities(std::vector<SceneEntity>& removingEntities);
        void restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation);

        SceneEntity getEntity(UUID uuid) const;

        bool isRunning() const { return m_running; }

        void setPath(const std::string& path) { m_path = path; }
        void setAssociatedProjectPath(const std::string& path) { m_associatedProjectPath = path;}

        const std::string& getAssociatedProjectPath() const {
            return m_associatedProjectPath;
        }

        [[maybe_unused]]
        SceneEntity duplicateEntity(robot2D::vec2f mousePos, SceneEntity entity);
        [[maybe_unused]]
        SceneEntity duplicateEmptyEntity(SceneEntity entity);

        [[maybe_unused]]
        SceneEntity duplicateRuntime(SceneEntity entity, robot2D::vec2f position);

        const std::string& getPath() const {return m_path;}
        std::string& getPath()  {return m_path;}

        void setEditorCamera(EditorCamera::Ptr editorCamera);

        bool hasChanges() const { return m_hasChanges; }

        /// \brief use for simple traverse inside Scene and don't think how SceneGraph works by outside caller.
        void traverseGraph(TraverseFunction&& traverseFunction);
    protected:
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
    private:
        struct RemoveEntityInfo {
            SceneEntity entity;
            bool isDeleted{false};
        };

        void initScene();
        void onPhysics2DRun(IScriptInteractorFrom::Ptr scriptInteractor);
        void onPhysics2DStop();

        void removeDuplicate(SceneEntity entity);
        void removeChildEntities(DeletedEntitiesRestoreInformation& information,
                                 std::vector<RemoveEntityInfo>& removingEntities,
                                 const SceneEntity& parent, bool isParentDel = false);

        void duplicateEntityChild(SceneEntity parent, SceneEntity dupEntity);

        void registerUICallback(SceneEntity uiEntity);
    private:
        friend class EditorLogic;
        friend class DuplicateCommand;
        SceneGraph m_sceneGraph;
        SceneGraph m_runtimeSceneGraph;
        std::vector<SceneEntity> m_scriptRuntimeContainer;

        std::vector<robot2D::ecs::Entity> m_runtimeClonedArray;
        std::list<SceneEntity> m_listPhysics;

        robot2D::ecs::Scene m_scene;
        robot2D::ecs::Scene m_runtimeScene;

        IEditorCamera::Ptr m_editorCamera{nullptr};
        IPhysics2DAdapter::Ptr m_physicsAdapter{ nullptr };

        robot2D::MessageBus& m_messageBus;
        std::string m_path;
        std::string m_associatedProjectPath;
        bool m_running = false;
        bool m_hasChanges{false};

        using Iterator = std::list<SceneEntity>::iterator;   
        using SetItem = std::tuple<Iterator, SceneEntity, bool, SceneEntity>;

        struct RestoreData {
            bool isChained{ false };
            Iterator anchorIterator;
            SceneEntity anchorEntity {};
            SceneEntity sourceEntity;
        };

        std::vector<RestoreData> m_restoreItems;
    };
} // namespace editor