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

#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/Util/ResourceHandler.hpp>
#include <robot2D/Graphics/Texture.hpp>

#include <editor/MessageDispather.hpp>
#include <editor/Messages.hpp>
#include <editor/PrefabManager.hpp>

#include "IPanel.hpp"
#include "TreeHierarchy.hpp"
#include <editor/UIInteractor.hpp>
#include <editor/DeletedEntitesRestoreInformation.hpp>

namespace editor {

    struct ScenePanelConfiguration {
        const int rightMouseButton = 1;
    };

    class ScenePanel: public IPanel {
    public:
        ScenePanel(robot2D::MessageBus& messageBus,
                   MessageDispatcher& messageDispatcher,
                   PrefabManager& prefabManager);
        ScenePanel(const ScenePanel& other) = delete;
        ScenePanel& operator=(const ScenePanel& other) = delete;
        ScenePanel(ScenePanel&& other) = delete;
        ScenePanel& operator=(ScenePanel&& other) = delete;
        ~ScenePanel() override = default;

        void setInteractor(UIInteractor::Ptr interactor);
        void render() override;

        robot2D::ecs::Entity getSelectedEntity() const;
        robot2D::ecs::Entity getTreeItem(UUID uuid);

        void clearSelection();

        void processSelectedEntities(std::vector<ITreeItem::Ptr>&& items);

        DeletedEntitiesRestoreUIInformation
        removeEntitiesOnUI(std::vector<ITreeItem::Ptr>&& uiItems);
        void restoreEntitiesOnUI(const DeletedEntitiesRestoreUIInformation& restoreUiInformation);
    private:
        void windowFunction();

        void onEntitySelection(const PanelEntitySelectedMessage& entitySelection);
        void onEntityDuplicate(const EntityDuplication& duplication);
        void onEntityRemove(const EntityRemovement& removement);

        void entityDuplicateChild(robot2D::ecs::Entity parentEntity, ITreeItem::Ptr parentItem);

        void setupTreeHierarchy();
        void setStartChildEntity(robot2D::ecs::Entity entity, ITreeItem::Ptr parent);
        void processSelectedChildren(ITreeItem::Ptr parent, std::vector<ITreeItem::Ptr>& items);
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        PrefabManager& m_prefabManager;

        UIInteractor::Ptr m_interactor;

        robot2D::ecs::Entity m_selectedEntity;
        ScenePanelConfiguration m_configuration;
        TreeHierarchy m_treeHierarchy;

        bool m_selectedEntityNeedCheckForDelete{false};

        enum class TreeItemIcon {
            Default,
            Prefab
        };
        robot2D::ResourceHandler<robot2D::Texture, TreeItemIcon> m_iconTextures;
    };
}