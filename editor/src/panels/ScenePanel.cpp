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

#include <filesystem>

#include <robot2D/imgui/Api.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


#include <editor/panels/ScenePanel.hpp>
#include <editor/Components.hpp>
#include <editor/Messages.hpp>

#include <editor/panels/ScenePanelUI.hpp>
#include <editor/Macro.hpp>

#include <editor/FileApi.hpp>
#include <editor/DragDropIDS.hpp>

namespace editor {

    #define GET_ENTITY_UUID(item) item -> getUserData<robot2D::ecs::Entity>() -> getComponent<IDComponent>().ID;
    #define GET_ENTITY(item) item -> getUserData<robot2D::ecs::Entity>()



    ScenePanel::ScenePanel(robot2D::MessageBus& messageBus,
                           MessageDispatcher& messageDispatcher,
                           PrefabManager& prefabManager):
    IPanel(UniqueType(typeid(ScenePanel))),
    m_messageBus{messageBus},
    m_messageDispatcher{messageDispatcher},
    m_prefabManager{prefabManager},
    m_interactor{nullptr},
    m_selectedEntity{},
    m_configuration{},
    m_treeHierarchy("MainScene")
    {

        if(!m_iconTextures.loadFromFile(TreeItemIcon::Default, "res/icons/entity.png")) {
            RB_EDITOR_ERROR("ScenePanel: Can't load TreeItemIcon::Default");
            /// TODO(a.raag) throw error
        }

        if(!m_iconTextures.loadFromFile(TreeItemIcon::Prefab, "res/icons/entity.png")) {
            RB_EDITOR_ERROR("ScenePanel: Can't load TreeItemIcon::Prefab");
            /// TODO(a.raag) throw error
        }

        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(
                MessageID::PanelEntityNeedSelect,
                BIND_CLASS_FN(onEntitySelection)
        );

        m_messageDispatcher.onMessage<EntityDuplication>(
                EntityDuplicate,
                BIND_CLASS_FN(onEntityDuplicate)
        );

        m_messageDispatcher.onMessage<EntityRemovement>(
                EntityRemove,
                BIND_CLASS_FN(onEntityRemove)
        );

        setupTreeHierarchy();
    }

    void ScenePanel::render() {
        if(m_interactor == nullptr)
            return;

        robot2D::WindowOptions windowOptions{};
        windowOptions.name = "ScenePanel";

        robot2D::createWindow(windowOptions, BIND_CLASS_FN(windowFunction));
    }



    void ScenePanel::windowFunction() {
        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow("ScenePanelEntityCreation", m_configuration.rightMouseButton))
        {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_interactor -> addEmptyEntity();
                auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                m_selectedEntity = m_interactor -> getEntities().back();
                item -> setName(m_interactor -> getEntities().back().getComponent<TagComponent>().getTag());
                item -> setUserData(m_selectedEntity);
                item -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
                m_treeHierarchy.setSelected(item);
            }

            ImGui::EndPopup();
        }

        m_treeHierarchy.render();

        /// \brief prefab processing
        if(ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::AcceptDragDropPayload(contentPrefabItemID);
            if(payload) {
                if(payload -> IsDataType(contentPrefabItemID)) {
                    const char* data = static_cast<const char*>(payload -> Data);
                    if(data) {
                        std::string payloadString = std::string(data, payload->DataSize);
                        const wchar_t* rawPath = (const wchar_t *) payloadString.c_str();
                        std::filesystem::path prefabPath = std::filesystem::path("assets") / rawPath;
                        auto realPrefabPath = combinePath(m_interactor -> getAssociatedProjectPath(), prefabPath.string());

                        if(auto prefab = m_prefabManager.findPrefab(realPrefabPath)) {
                            auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                            robot2D::ecs::Entity duplicateEntity = m_interactor -> duplicateEmptyEntity(prefab -> entity);
                            item -> setName(duplicateEntity.getComponent<TagComponent>().getTag());
                            item -> setUserData(duplicateEntity);
                            item -> setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
                            m_treeHierarchy.setSelected(item);
                        }
                        else {

                            if(auto prefab = m_prefabManager.loadPrefab(m_interactor,
                                                                        realPrefabPath)) {
                                auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                                robot2D::ecs::Entity duplicateEntity = m_interactor -> duplicateEmptyEntity(prefab -> entity);
                                item -> setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
                                item -> setName(duplicateEntity.getComponent<TagComponent>().getTag());
                                item -> setUserData(duplicateEntity);
                                m_treeHierarchy.setSelected(item);
                            }
                        }

                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
            m_selectedEntity = {};
            auto* msg = m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected);
            msg -> entity = m_selectedEntity;
        }
    }

    void ScenePanel::setupTreeHierarchy() {

        m_treeHierarchy.addOnSelectCallback([this](ITreeItem::Ptr selectedItem) {
            auto entity = selectedItem -> getUserData<robot2D::ecs::Entity>();
            if(entity) {
                m_selectedEntity = *entity;
                auto* msg = m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected);
                msg -> entity = m_selectedEntity;
            }
            else {
                RB_EDITOR_ERROR("Selected item in Hierarchy don't have userData");
            }
        });

        m_treeHierarchy.addOnCallback([this](ITreeItem::Ptr item) {

            bool deleteSelected = false;
            if (ImGui::BeginPopupContextItem("TreeHierarchyOnCallbackPopup"))
            {
                if (ImGui::MenuItem("Delete Entity"))
                    deleteSelected = true;

                ImGui::EndPopup();
            }

            if(deleteSelected) {
                if(item -> isChild()) {
                    auto entity = item -> getUserData<robot2D::ecs::Entity>();
                    if(entity) {
                        auto& transform = entity -> getComponent<TransformComponent>();
                        if(transform.isChild())
                            transform.removeSelf();
                        /// delete child command ///
                    }
                    item -> removeSelf();
                }
                else {
                    if(item -> hasChildrens()) {
                        for(auto child: item -> getChildrens()) {
                            auto entity = child -> getUserData<robot2D::ecs::Entity>();
                            m_interactor -> removeEntity(std::move(*entity));
                            /// delete child command ///
                            item -> deleteChild(child);
                        }
                        auto entity = item -> getUserData<robot2D::ecs::Entity>();
                        m_interactor -> removeEntity(std::move(*entity));
                        m_treeHierarchy.deleteItem(item);
                    }
                    else {
                        auto entity = item -> getUserData<robot2D::ecs::Entity>();
                        if(!entity) {
                            RB_EDITOR_ERROR("Cant in Hierarchy don't have userData");
                            return;
                        }
                        /// delete command ///
                        m_interactor -> removeEntity(*entity);
                        m_treeHierarchy.deleteItem(item);
                    }
                }
                m_selectedEntity = {};
                m_selectedEntityNeedCheckForDelete = true;
            }
        });

        m_treeHierarchy.addOnReorderCallback([this](ITreeItem::Ptr source, ITreeItem::Ptr target){
            auto sourceEntity = source -> getUserData<robot2D::ecs::Entity>();
            if(!sourceEntity) {
                RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
                return;
            }

            if(sourceEntity -> hasComponent<DrawableComponent>()) {
                auto& drawable = sourceEntity -> getComponent<DrawableComponent>();
                drawable.setReorderZBuffer(true);
            }

            m_interactor -> setBefore(*sourceEntity, *target -> getUserData<robot2D::ecs::Entity>());
            m_treeHierarchy.setBefore(source, target);
        });

        m_treeHierarchy.addOnMakeChildCallback([this](ITreeItem::Ptr source, ITreeItem::Ptr intoTarget) {
           auto entity = intoTarget -> getUserData<robot2D::ecs::Entity>();
           if(!entity) {
               RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
               return;
           }
           auto& transform = entity -> getComponent<TransformComponent>();
           auto sourceEntity = source -> getUserData<robot2D::ecs::Entity>();
           if(!sourceEntity) {
               RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
               return;
           }

           if(sourceEntity -> hasComponent<DrawableComponent>()) {
               auto& sprite = sourceEntity -> getComponent<DrawableComponent>();
               sprite.setReorderZBuffer(true);
           }

           transform.addChild(*sourceEntity);
            m_interactor -> removeEntityChild(*sourceEntity);
           m_treeHierarchy.deleteItem(source);
           intoTarget -> addChild(source);
        });

        m_treeHierarchy.addOnStopBeChildCallback([this](ITreeItem::Ptr source, ITreeItem::Ptr intoTarget) {
            auto entity = source -> getUserData<robot2D::ecs::Entity>();
            if(!entity) { return; }
            auto& ts = entity -> getComponent<TransformComponent>();
            bool needRemoveFromScene = false;
            ts.removeSelf(needRemoveFromScene);
            source -> removeSelf();

            bool needPendingAppend = true;
            auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>(needPendingAppend);
            item -> setName(entity -> getComponent<TagComponent>().getTag());
            item -> setUserData(*entity);
            ///TODO(@a.raag) set before ??
            //m_treeHierarchy.setBefore()
        });

        m_treeHierarchy.addMultiSelectCallback([this](std::vector<ITreeItem::Ptr> items) {
            RB_EDITOR_INFO("TreeHierarchy: MultiSelect All");
            ITreeItem::Ptr item;
            auto uuid = GET_ENTITY_UUID(item)
        });

        m_treeHierarchy.addMultiSelectRangeCallback([this](std::vector<ITreeItem::Ptr> items, bool deleted) {
            /// TODO(a.raag): Filter by items
        });
    }

    void ScenePanel::setInteractor(UIInteractor::Ptr interactor) {
        if(interactor == nullptr)
            return;
        m_interactor = interactor;

        m_interactor -> registerOnDeleteFinish([this]() {
           m_selectedEntityNeedCheckForDelete = false;
        });

        m_selectedEntity = {};
        m_treeHierarchy.clear();


        for(auto entity: m_interactor -> getEntities()) {
            auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
            item -> setName(entity.getComponent<TagComponent>().getTag());
            item -> setUserData(entity);
            item -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
            auto& transform = entity.getComponent<TransformComponent>();
            if(transform.hasChildren()) {
                for(auto& child: transform.getChildren()) {
                    auto childItem = item -> addChild();
                    childItem -> setName(child.getComponent<TagComponent>().getTag());
                    childItem -> setUserData(child);
                    childItem -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
                }
            }
        }

    }

    robot2D::ecs::Entity ScenePanel::getSelectedEntity(int PixelData) {

        for(auto& item: m_treeHierarchy.getItems()) {
            auto entityPtr = item -> getUserData<robot2D::ecs::Entity>();
            if(!entityPtr)
                return {};
            robot2D::ecs::Entity entity = *entityPtr;

            if(entity.getIndex() == PixelData) {
                m_selectedEntity = entity;
                m_treeHierarchy.setSelected(item);
                return m_selectedEntity;
            }

            if(item -> hasChildrens()) {
                for(auto child: item -> getChildrens()) {
                    entityPtr = child -> getUserData<robot2D::ecs::Entity>();
                    if(!entityPtr)
                        return {};
                    entity = *entityPtr;
                    if(entity.getIndex() == PixelData) {
                        m_selectedEntity = entity;
                        m_treeHierarchy.setSelected(child);
                        return m_selectedEntity;
                    }
                }
            }
        }
        return {};
    }

    robot2D::ecs::Entity ScenePanel::getSelectedEntity() const {
        return m_selectedEntity;
    }

    robot2D::ecs::Entity ScenePanel::getTreeItem(UUID uuid) {
        return *m_treeHierarchy.getDataByItem<robot2D::ecs::Entity>(uuid);
    }


    void ScenePanel::onEntitySelection(const PanelEntitySelectedMessage& entitySelection) {
        for(auto& item: m_treeHierarchy.getItems()) {
            auto entityPtr = item -> getUserData<robot2D::ecs::Entity>();
            if(!entityPtr)
                return;
            robot2D::ecs::Entity entity = *entityPtr;

            if(entity == entitySelection.entity) {
                m_selectedEntity = entity;
                m_treeHierarchy.setSelected(item);
            }

            if(item -> hasChildrens()) {
                for(auto child: item -> getChildrens()) {
                    entityPtr = child -> getUserData<robot2D::ecs::Entity>();
                    if(!entityPtr)
                        return;
                    entity = *entityPtr;
                    if(entity == entitySelection.entity) {
                        m_selectedEntity = entity;
                        m_treeHierarchy.setSelected(child);
                    }
                }
            }
        }
    }

    void ScenePanel::onEntityDuplicate(const EntityDuplication& duplication) {
        auto entity = m_interactor -> getByUUID(duplication.entityID);
        auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
        item -> setName(entity.getComponent<TagComponent>().getTag());
        item -> setUserData(entity);

        auto& transform = entity.getComponent<TransformComponent>();
        if(transform.hasChildren()) {
            for(auto& child: transform.getChildren()) {
                auto childItem = item -> addChild();
                childItem -> setName(child.getComponent<TagComponent>().getTag());
                childItem -> setUserData(child);
            }
        }
    }

    void ScenePanel::onEntityRemove(const EntityRemovement& removement) {
        for(auto& item: m_treeHierarchy.getItems()) {
            if(item -> getUserData<robot2D::ecs::Entity>() -> getComponent<IDComponent>().ID == removement.entityID) {
                m_treeHierarchy.deleteItem(item);
                return;
            }
        }
    }

    void ScenePanel::processSelectedEntities(std::vector<robot2D::ecs::Entity>& entities) {
        auto& treeItems = m_treeHierarchy.getItems();
        for(auto& entity: entities) {
            for(auto& treeItem: treeItems) {
                auto treeEntity = treeItem -> getUserData<robot2D::ecs::Entity>();
                if(!treeEntity) {
                    RB_EDITOR_ERROR("ScenePanel: TreeItem By UUID = {0} don't have Entity");
                    continue;
                }

                if(entity == *treeEntity) {
                    m_treeHierarchy.setSelected(treeItem);
                }
            }
        }
    }

    DeletedEntitiesRestoreUIInformation
    ScenePanel::removeEntitiesOnUI(std::vector<robot2D::ecs::Entity>& selectedEntities) {
        DeletedEntitiesRestoreUIInformation restoreUiInformation;
        auto& uiItems = m_treeHierarchy.getItems();

        for(auto entity: selectedEntities) {
            for(auto uiIter = uiItems.begin(); uiIter < uiItems.end(); ++uiIter) {
                auto uiEntity = GET_ENTITY((*uiIter));
                if(!uiEntity -> destroyed() && *uiEntity == entity) {
                    if(!m_treeHierarchy.deleteItem(*uiIter)) {
                        RB_EDITOR_ERROR("TreeHierarchy can't delete item.", (*uiIter) -> getID());
                    }

                    if(uiIter == uiItems.begin())
                        restoreUiInformation.push(*uiIter, *uiIter, true);
                    else {
                        auto prev = uiIter - 1;
                        restoreUiInformation.push(*uiIter, *prev, false);
                    }
                }
            }
        }

        return restoreUiInformation;
    }

    void ScenePanel::restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation& restoreUiInformation) {
        for(auto& item: restoreUiInformation.anchorItems) {
            m_treeHierarchy.insertItem(item.target, item.anchor, item.first);
        }
    }

    void ScenePanel::clearSelection() {
        m_treeHierarchy.clearSelection();
    }


}