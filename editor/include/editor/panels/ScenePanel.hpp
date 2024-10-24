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

#include<list>

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
#include <editor/SceneEntity.hpp>

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
        SceneEntity getTreeItem(UUID uuid);

        void processSelectedEntities(std::vector<ITreeItem::Ptr>&& items);
        void clearSelection();

        DeletedEntitiesRestoreUIInformation removeEntitiesOnUI(std::list<ITreeItem::Ptr>&& uiItems);
        void restoreEntitiesOnUI(const DeletedEntitiesRestoreUIInformation& restoreUiInformation);

        void render() override;
    private:
        void windowFunction();

        void onEntityDuplicate(const EntityDuplication& duplication);
        void entityDuplicateChild(SceneEntity parentEntity, ITreeItem::Ptr parentItem);

        void setupTreeHierarchy();
        void setStartChildEntity(SceneEntity entity, ITreeItem::Ptr parent);

        void onSelectUIItem(ITreeItem::Ptr item);
        void onReorderUIItems(ITreeItem::Ptr source, ITreeItem::Ptr target);
        void onMakeChildUIItems(ITreeItem::Ptr source, ITreeItem::Ptr target);
        void onRemoveChildUIItems(ITreeItem::Ptr source, ITreeItem::Ptr target);
        void onMultiSelect(std::set<ITreeItem::Ptr>& items, bool allSelected);
    private:
        robot2D::MessageBus& m_messageBus;
        MessageDispatcher& m_messageDispatcher;
        PrefabManager& m_prefabManager;

        UIInteractor::Ptr m_interactor { nullptr };

        SceneEntity m_selectedEntity;
        ScenePanelConfiguration m_configuration;
        TreeHierarchy m_treeHierarchy;
    };
}