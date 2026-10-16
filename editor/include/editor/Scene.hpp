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
#include <unordered_map>

#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/Drawable.hpp>

#include "Components.hpp"
#include "physics/IPhysics2DAdapter.hpp"
#include "ScriptInteractor.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "EditorCamera.hpp"
#include "SceneEntity.hpp"

namespace editor {

    class Scene : public robot2D::Drawable {
    public:
        using Ptr = std::shared_ptr<Scene>;
        using TraverseFunction = std::function<void (SceneEntity&)>;

    public:
        explicit Scene(robot2D::MessageBus& messageBus);
        Scene(const Scene& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(Scene&& other) = delete;
        ~Scene() override = default;

        void createMainCamera();

        const std::list<SceneEntity>& getEntities() const;
        const std::list<SceneEntity>& getRootEntities(SceneContext context = SceneContext::Editor) const;
        std::vector<SceneEntity> getChildren(SceneEntity entity) const;
        SceneEntity getParent(SceneEntity entity) const;

        void handleEventsRuntime(const robot2D::Event& event);
        void update(float dt);
        void updateRuntime(float dt, IScriptInteractorFrom::Ptr scriptInteractor);

        void onRuntimeStart(IScriptInteractorFrom::Ptr scriptInteractor);
        void onRuntimeStop(IScriptInteractorFrom::Ptr scriptInteractor);

        void setRuntimeCamera(bool flag);
        void setRuntimeWindowSize(const robot2D::vec2u& size);

        // Creates a registered root with ID, Tag, Transform and Hierarchy components.
        SceneEntity createEntity(const std::string& name = "Untitled Entity");
        // Registers an entity after its components have been loaded.
        bool attachEntity(SceneEntity& entity);
        // Creates an unregistered entity for serializers and prefab construction.
        SceneEntity createEmptyEntity();

        /////////////////////////// ScenePanel API ///////////////////////////
        SceneEntity addEmptyEntity();
        SceneEntity addEmptyButton();
        void registerUICallback(SceneEntity uiEntity);
        /////////////////////////// ScenePanel API ///////////////////////////


        bool setBefore(SceneEntity source, SceneEntity target);
        bool reparent(SceneEntity entity, SceneEntity parent = {});
        DeletedEntitiesRestoreInformation removeEntities(std::vector<SceneEntity>& removingEntities);
        void restoreEntities(DeletedEntitiesRestoreInformation& restoreInformation);

        SceneEntity getEntity(UUID uuid) const;
        SceneEntity getEntity(UUID uuid, SceneContext context) const;
        SceneEntity getEntity(const std::string& name) const;

        bool isRunning() const {
            return m_running;
        }

        void setPath(const std::string& path) {
            m_path = path;
        }
        void setAssociatedProjectPath(const std::string& path) {
            m_associatedProjectPath = path;
        }

        const std::string& getAssociatedProjectPath() const {
            return m_associatedProjectPath;
        }

        [[maybe_unused]]
        SceneEntity duplicateEntity(robot2D::vec2f mousePos, SceneEntity entity);
        [[maybe_unused]]
        SceneEntity duplicateEmptyEntity(SceneEntity entity, IScriptInteractorFrom::Ptr scriptInteractor = {});

        // Materialize persisted template data in the active context. Runtime lifecycle
        // (physics bodies, managed instances/callbacks) belongs to the runtime caller.
        SceneEntity instantiateTemplate(SceneEntity source, IScriptInteractorFrom::Ptr scriptInteractor = {});

        [[maybe_unused]]
        SceneEntity duplicateRuntime(SceneEntity entity, robot2D::vec2f position);

        const std::string& getPath() const {
            return m_path;
        }
        std::string& getPath() {
            return m_path;
        }

        void setEditorCamera(EditorCamera::Ptr editorCamera);

        bool hasChanges() const {
            return m_hasChanges;
        }
        void markModified() {
            if(!m_running) m_hasChanges = true;
        }
        void markSaved() {
            m_hasChanges = false;
        }
        void setName(const std::string& name) {
            if(m_name != name) {
                m_name = name;
                markModified();
            }
        }
        const std::string& getName() const {
            return m_name;
        }

        void forEachEntity(TraverseFunction&& traverseFunction,
            SceneContext context = SceneContext::Editor);
    protected:
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
    private:
        void initScene();
        void onPhysics2DRun(IScriptInteractorFrom::Ptr scriptInteractor);
        void onPhysics2DStop();

        void removeDuplicate(SceneEntity entity);
        void duplicateEntityChild(SceneEntity parent, SceneEntity dupEntity);

        struct EntityStorage {
            EntityStorage() = default;
            EntityStorage(const EntityStorage& other) = delete;
            EntityStorage& operator=(const EntityStorage& other) = delete;
            EntityStorage(EntityStorage&& other) = default;
            EntityStorage& operator=(EntityStorage&& other) = default;
            ~EntityStorage() = default;

            std::list<SceneEntity> roots;
            std::unordered_map<UUID, SceneEntity> entities;
            std::vector<SceneEntity> pendingRemovals;
        };
        EntityStorage& getStorage(SceneContext context);
        const EntityStorage& getStorage(SceneContext context) const;
        SceneEntity wrapEntity(robot2D::ecs::Entity entity, SceneContext context);
        bool registerEntity(SceneEntity entity);
        void addRoot(SceneEntity entity);
        bool ownsEntity(SceneEntity entity) const;
        bool isContextValid(SceneContext context, std::uint64_t version) const;
        bool reorderEntity(SceneEntity source, SceneEntity target);
        void queueRemoval(SceneEntity entity);
        void processRemovals(SceneContext context);
        void cancelPendingRemoval(SceneEntity entity);
        void moveChildren(UUID parentID, const robot2D::vec2f& offset,
            SceneContext context, std::uint64_t version);
        void forEachChild(TraverseFunction& traverseFunction, SceneEntity parent);
    private:
        friend class SceneEntity;
        friend class TransformComponent;
        friend class DuplicateCommand;
        EntityStorage m_editorEntities;
        EntityStorage m_runtimeEntities;
        std::uint64_t m_runtimeVersion { 0 };
        std::vector<SceneEntity> m_scriptRuntimeContainer;

        std::vector<robot2D::ecs::Entity> m_runtimeClonedArray;
        std::list<SceneEntity> m_listPhysics;

        robot2D::ecs::Scene m_scene;
        std::unique_ptr<robot2D::ecs::Scene> m_runtimeScene;

        IEditorCamera::Ptr m_editorCamera{nullptr};
        IPhysics2DAdapter::Ptr m_physicsAdapter{ nullptr };

        robot2D::MessageBus& m_messageBus;
        std::string m_path;
        std::string m_name = "Unnamed Scene";
        std::string m_associatedProjectPath;
        bool m_running = false;
        bool m_hasChanges{false};

    };
} // namespace editor
