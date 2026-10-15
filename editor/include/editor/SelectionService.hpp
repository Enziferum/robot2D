#pragma once
#include <optional>
#include <vector>
#include "Uuid.hpp"

namespace editor {

    class SelectionService {
    public:
        SelectionService() = default;
        SelectionService(const SelectionService& other) = delete;
        SelectionService& operator=(const SelectionService& other) = delete;
        SelectionService(SelectionService&& other) = delete;
        SelectionService& operator=(SelectionService&& other) = delete;
        ~SelectionService() = default;

        void selectEntity(UUID uuid);
        void selectEntities(const std::vector<UUID>& entities,
                            std::optional<UUID> active = std::nullopt);
        void addEntity(UUID uuid);
        void removeEntity(UUID uuid);
        void toggleEntity(UUID uuid);
        void clear();

        bool hasEntity(UUID uuid) const;
        bool empty() const;
        const std::vector<UUID>& getSelectedEntities() const;
        std::optional<UUID> lastActive() const;
    private:
        std::vector<UUID> m_selectedEntities;
        std::optional<UUID> m_activeEntity;
    };

}
