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
#include <editor/panels/ScenePanel.hpp>
#include <editor/Components.hpp>
#include <editor/Messages.hpp>

#include <editor/panels/ScenePanelUI.hpp>
#include <editor/Macro.hpp>

#include <editor/FileApi.hpp>
#include <editor/DragDropIDS.hpp>

namespace editor {

    #define GET_ENTITY_UUID(item) item -> getUserData<robot2D::ecs::Entity>() -> getComponent<IDComponent>().ID;
    #define GET_ENTITY(item) item -> template getUserData<robot2D::ecs::Entity>()



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
                MessageID::EntityDuplicate,
                BIND_CLASS_FN(onEntityDuplicate)
        );

        m_messageDispatcher.onMessage<EntityRemovement>(
                MessageID::EntityRemove,
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

        imgui_PopupContextWindow("ScenePanelEntityCreation",
                                 m_configuration.rightMouseButton) {

            imgui_MenuItem("Create Entity") {
                m_interactor -> addEmptyEntity();
                auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                m_selectedEntity = m_interactor -> getEntities().back();
                item -> setName(&m_interactor -> getEntities().back().getComponent<TagComponent>().getTag());
                item -> setUserData(m_selectedEntity);
                item -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
                m_selectedEntity.addComponent<UIComponent>().treeItem = item;
                m_treeHierarchy.setSelected(item);
            }

            imgui_Menu("Create UI Elemement") {
                imgui_MenuItem("Button") {
                    m_selectedEntity = m_interactor -> addButton();
                    auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                    item -> setName(&m_selectedEntity.getComponent<TagComponent>().getTag());
                    item -> setUserData(m_selectedEntity);
                    item -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
                    m_selectedEntity.addComponent<UIComponent>().treeItem = item;
                    m_treeHierarchy.setSelected(item);
                }
            }
        }

        m_treeHierarchy.render();

        /// \brief prefab processing
        {
            robot2D::DragDropTarget dragDropTarget{ contentPrefabItemID };
            if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                auto&& path = payloadBuffer.unpack<std::string>();
                std::filesystem::path prefabPath = std::filesystem::path("assets") / path;
                auto realPrefabPath = combinePath(m_interactor -> getAssociatedProjectPath(),
                                                  prefabPath.string());

                if(auto prefab = m_prefabManager.loadPrefab(m_interactor, realPrefabPath)) {
                    auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                    robot2D::ecs::Entity duplicateEntity = m_interactor -> duplicateEmptyEntity(prefab -> entity);
                    item -> setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
                    item -> setName(&duplicateEntity.getComponent<TagComponent>().getTag());
                    item -> setUserData(duplicateEntity);
                    duplicateEntity.addComponent<UIComponent>().treeItem = item;
                    m_treeHierarchy.setSelected(item);
                }
            }
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
                auto* msg =
                        m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected);
                msg -> entity = m_selectedEntity;
            }
            else {
                RB_EDITOR_ERROR("Selected item in Hierarchy don't have userData");
            }
        });

        m_treeHierarchy.addOnCallback([this](ITreeItem::Ptr item) {

            bool deleteSelected = false;
            if (ImGui::BeginPopupContextItem("TreeHierarchyDeletePopup"))
            {
                if (ImGui::MenuItem("Delete Entity"))
                    deleteSelected = true;

                ImGui::EndPopup();
            }
            // TODO(a.raag): correct deletion from UI
            if(deleteSelected) {
//                const auto& selectedItems = m_treeHierarchy.getSelectedItems();
//                m_interactor -> removeEntities(selectedItems);
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
            RB_EDITOR_WARN("m_treeHierarchy.addOnMakeChildCallback");
            auto entity = intoTarget -> getUserData<robot2D::ecs::Entity>();
            if(!entity) {
                RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
                return;
            }
            auto& transform = entity -> getComponent<TransformComponent>();

            auto sourceEntity = source -> getUserData<robot2D::ecs::Entity>();
            if(!sourceEntity) {
                RB_EDITOR_ERROR("ScenePanel: Can't item in Hierarchy don't have userData");
                return;
            }

            if(sourceEntity -> hasComponent<DrawableComponent>()) {
                auto& sprite = sourceEntity -> getComponent<DrawableComponent>();
                sprite.setReorderZBuffer(true);
            }

            if(source -> isChild()) {
                auto& sourceTransform = sourceEntity -> getComponent<TransformComponent>();
                const bool needRemoveFromScene = false;
                sourceTransform.removeSelf(needRemoveFromScene);
                source -> removeSelf();
                transform.addChild(*entity, *sourceEntity);
                m_treeHierarchy.applyChildModification(source, intoTarget);
            }
            else {
                transform.addChild(*entity, *sourceEntity);
                m_interactor -> removeEntityChild(*sourceEntity);
                m_treeHierarchy.deleteItem(source);
                intoTarget -> addChild(source);
            }

        });

        /// TODO(a.raag): Fix Logic
        m_treeHierarchy.addOnStopBeChildCallback([this](ITreeItem::Ptr source, ITreeItem::Ptr intoTarget) {
            auto entity = GET_ENTITY(source);
            if(!entity || entity -> destroyed()) {
                RB_EDITOR_ERROR("");
                return;
            }

            auto& ts = entity -> getComponent<TransformComponent>();
            bool needRemoveFromScene = false;
            ts.removeSelf(needRemoveFromScene);
            source -> removeSelf();

            auto targetEntity = GET_ENTITY(intoTarget);
            if(!targetEntity || targetEntity -> destroyed()) {
                RB_EDITOR_ERROR("");
                return;
            }

            targetEntity -> getComponent<TransformComponent>().addChild(*targetEntity, *entity);

            if(intoTarget -> isChild())
                intoTarget -> addChild(source);
            else {
                ///
            }

            m_treeHierarchy.applyChildModification(source, intoTarget);
        });


        m_treeHierarchy.addMultiSelectCallback([this](std::set<ITreeItem::Ptr>& items, bool allSelected) {
            m_interactor -> uiSelectedEntities(items, allSelected);
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
            item -> setName(&entity.getComponent<TagComponent>().getTag());
            item -> setUserData(entity);
            if(entity.hasComponent<PrefabComponent>())
                item -> setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
            else
                item -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
            entity.addComponent<UIComponent>().treeItem = item;

            auto& transform = entity.getComponent<TransformComponent>();
            if(transform.hasChildren())
                setStartChildEntity(entity, item);
        }

    }

    void ScenePanel::setStartChildEntity(robot2D::ecs::Entity entity, ITreeItem::Ptr parent) {
        auto& parentTransform = entity.getComponent<TransformComponent>();
        if(auto item = std::dynamic_pointer_cast<TreeItem<robot2D::ecs::Entity>>(parent)) {
            for(auto& child: parentTransform.getChildren()) {
                auto childItem = item -> addChild();
                childItem -> setName(&child.getComponent<TagComponent>().getTag());
                childItem -> setUserData(child);
                if(child.hasComponent<PrefabComponent>())
                    childItem -> setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
                else
                    childItem -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
                child.addComponent<UIComponent>().treeItem = childItem;
                auto& childTransform = child.getComponent<TransformComponent>();
                if(childTransform.hasChildren())
                    setStartChildEntity(child, childItem);
            }
        }
    }

    robot2D::ecs::Entity ScenePanel::getSelectedEntity() const {
        return m_selectedEntity;
    }

    robot2D::ecs::Entity ScenePanel::getTreeItem(UUID uuid) {
        return *m_treeHierarchy.getDataByItem<robot2D::ecs::Entity>(uuid);
    }


    void ScenePanel::onEntitySelection(const PanelEntitySelectedMessage& message) {

        /// TODO(a.raag): get UUID or ITreeItem::Ptr directly and process whole children tree

        for(auto& item: m_treeHierarchy.getItems()) {
            auto entityPtr = item -> getUserData<robot2D::ecs::Entity>();
            if(!entityPtr)
                return;
            robot2D::ecs::Entity entity = *entityPtr;

            if(entity == message.entity) {
                m_selectedEntity = entity;
                m_treeHierarchy.setSelected(item);
            }

            if(item -> hasChildrens()) {
                for(auto child: item -> getChildrens()) {
                    entityPtr = child -> getUserData<robot2D::ecs::Entity>();
                    if(!entityPtr)
                        return;
                    entity = *entityPtr;
                    if(entity == message.entity) {
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
        item -> setName(&entity.getComponent<TagComponent>().getTag());
        item -> setUserData(entity);
        if(entity.hasComponent<PrefabComponent>())
            item -> setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
        else
            item -> setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
        entity.addComponent<UIComponent>().treeItem = item;

        auto& transform = entity.getComponent<TransformComponent>();
        if(transform.hasChildren()) {
            entityDuplicateChild(entity, item);
        }
    }

    void ScenePanel::entityDuplicateChild(robot2D::ecs::Entity parentEntity, ITreeItem::Ptr parentItem) {
        auto& transform = parentEntity.getComponent<TransformComponent>();
        if(auto item = std::dynamic_pointer_cast<TreeItem<robot2D::ecs::Entity>>(parentItem)) {
            for (auto &child: transform.getChildren()) {
                auto childItem = item -> addChild();
                childItem -> setName(&child.getComponent<TagComponent>().getTag());
                childItem -> setUserData(child);
                if (child.hasComponent<PrefabComponent>())
                    childItem->setTexture(m_iconTextures[TreeItemIcon::Prefab], robot2D::Color::Blue);
                else
                    childItem->setTexture(m_iconTextures[TreeItemIcon::Default], robot2D::Color::White);
                child.addComponent<UIComponent>().treeItem = childItem;

                if(child.getComponent<TransformComponent>().hasChildren())
                    entityDuplicateChild(child, childItem);
            }
        }
    }

    void ScenePanel::onEntityRemove(const EntityRemovement& removement) {
        /// TODO(a.raag): get UUID or ITreeItem::Ptr directly

        for(auto& item: m_treeHierarchy.getItems()) {
            if(item -> getUserData<robot2D::ecs::Entity>() -> getComponent<IDComponent>().ID == removement.entityID) {
                m_treeHierarchy.deleteItem(item);
                return;
            }
        }
    }


    void ScenePanel::processSelectedEntities(std::vector<ITreeItem::Ptr>&& items) {
        for(auto& item: items)
            m_treeHierarchy.setSelected(item);
    }

    void ScenePanel::processSelectedChildren(ITreeItem::Ptr parent, std::vector<ITreeItem::Ptr>& items) {
        for(auto& item: items) {
            for(auto& childItem: parent -> getChildrens()) {
                if(item == childItem)
                    m_treeHierarchy.setSelected(childItem);

                if(childItem -> hasChildrens())
                    processSelectedChildren(childItem, items);
            }
        }
    }


    DeletedEntitiesRestoreUIInformation
    ScenePanel::removeEntitiesOnUI(std::vector<ITreeItem::Ptr>&& uiItems) {
        DeletedEntitiesRestoreUIInformation restoreUiInformation;
        bool removeAll = uiItems.size() == m_treeHierarchy.getItems().size();

        if(removeAll) {
            m_treeHierarchy.getItems();
            auto& items = m_treeHierarchy.getItems();
            auto iter = items.begin();
            {
                DeletedEntitiesRestoreUIInformation::RestoreInfo restoreInfo;
                restoreInfo.first = true;
                restoreInfo.target = *iter;
                m_treeHierarchy.deleteItem(*iter);
                restoreUiInformation.push(std::move(restoreInfo));
            }
            if(items.size() > 1) {
                for(auto it = std::next(iter); it != items.end(); ++it) {
                    DeletedEntitiesRestoreUIInformation::RestoreInfo restoreInfo;
                    restoreInfo.isChained = true;
                    restoreInfo.anchor = *std::prev(it);
                    restoreInfo.target = *it;
                    m_treeHierarchy.deleteItem(*it);
                    restoreUiInformation.push(std::move(restoreInfo));
                }
            }
        }
        else {
            for(const auto& item: uiItems) {

                auto found = std::find_if(uiItems.begin(), uiItems.end(),
                                          [&item](const ITreeItem::Ptr& obj) {
                    return *item == *obj;
                });

                if(found == uiItems.end())
                    continue;

                m_treeHierarchy.deleteItem(*found);

                DeletedEntitiesRestoreUIInformation::RestoreInfo restoreInfo{};
                const auto& prev = std::prev(found);
                restoreInfo.target = *found;

                if(!restoreUiInformation.hasItems()) {
                    if(found == uiItems.begin())
                        restoreInfo.first = true;
                    else
                        restoreInfo.anchor = *prev;
                }
                else {
                    const auto& lastInfo = restoreUiInformation.getLast();
                    if(lastInfo.target == *prev) {
                        restoreInfo.isChained = true;
                        restoreInfo.anchor = lastInfo.target;
                    }
                    else {
                        if(found == uiItems.begin())
                            restoreInfo.first = true;
                        else
                            restoreInfo.anchor = *prev;
                    }
                }

                restoreUiInformation.push(std::move(restoreInfo));
            }
        }

        return restoreUiInformation;
    }

    void ScenePanel::restoreEntitiesOnUI(const DeletedEntitiesRestoreUIInformation& restoreUiInformation) {
        for(const auto& item: restoreUiInformation.getItems()) {
            TreeHierarchy::RestoreInfo hierarchyRestoreInfo;
            hierarchyRestoreInfo.target = item.target;
            hierarchyRestoreInfo.anchor = item.anchor;
            hierarchyRestoreInfo.isChained = item.isChained;
            hierarchyRestoreInfo.isFirst = item.first;
            m_treeHierarchy.restoreItem(std::move(hierarchyRestoreInfo));
        }
    }


    void ScenePanel::clearSelection() {
        m_treeHierarchy.clearSelection();
    }

}