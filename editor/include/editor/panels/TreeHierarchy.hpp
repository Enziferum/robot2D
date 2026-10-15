/*********************************************************************
(c) Alex Raag 2026
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

#include <functional>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <editor/SceneEntity.hpp>
#include <editor/SelectionService.hpp>

namespace editor {

    class TreeHierarchy {
    public:
        using EntitySource = std::function<const std::list<SceneEntity>&()>;
        using SelectionCallback = std::function<void()>;
        using ReorderCallback = std::function<void(UUID source, UUID target)>;

        explicit TreeHierarchy(std::string name);
        TreeHierarchy(const TreeHierarchy& other) = delete;
        TreeHierarchy& operator=(const TreeHierarchy& other) = delete;
        TreeHierarchy(TreeHierarchy&& other) = delete;
        TreeHierarchy& operator=(TreeHierarchy&& other) = delete;
        ~TreeHierarchy() = default;

        void setEntitySource(EntitySource&& source);
        void setSelectionService(SelectionService& selectionService);
        void setSelectionCallback(SelectionCallback&& callback);
        void setReorderCallback(ReorderCallback&& callback);
        void setMakeChildCallback(ReorderCallback&& callback);

        void setExpanded(UUID uuid, bool expanded);
        bool isExpanded(UUID uuid) const;
        const std::vector<UUID>& getVisibleEntities() const { return m_visibleEntities; }

        void clearSelection();
        void clear();
        void render();
    private:
        struct NodeState {
            bool expanded { true };
        };

        enum class DropAction {
            Reorder,
            MakeChild
        };

        struct PendingDrop {
            UUID source;
            UUID target;
            DropAction action;
        };

        void renderEntity(SceneEntity entity);
        void processSelection();
        void processDrops();
        void notifySelection();
    private:
        std::string m_name;
        EntitySource m_entitySource;
        SelectionService* m_selectionService { nullptr };
        SelectionCallback m_selectionCallback;
        ReorderCallback m_reorderCallback;
        ReorderCallback m_makeChildCallback;

        // UI state only. Names, parent links and ordering are read from the scene.
        // Keep deleted UUIDs until the scene changes so restoration retains expansion.
        std::unordered_map<UUID, NodeState> m_nodeStates;
        bool m_rootExpanded { true };
        std::optional<UUID> m_rangeAnchor;
        std::optional<UUID> m_clickedEntity;
        std::vector<UUID> m_visibleEntities;
        std::vector<PendingDrop> m_pendingDrops;
    };
}
