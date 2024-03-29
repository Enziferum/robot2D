#pragma once
#include <vector>
#include <yaml-cpp/yaml.h>

#include <robot2D/Ecs/Entity.hpp>
#include <editor/Uuid.hpp>

namespace editor {
    using ChildPair = std::pair<UUID, std::vector<UUID>>;

    struct ChildInfo {
        bool isChild{false};
        robot2D::ecs::Entity self;
        UUID parentUUID;
        ChildPair childPair;

        bool isEmpty() const { return !isChild && childPair.second.empty(); }
        bool hasChildren() const { return !childPair.second.empty(); }
    };

    class EntitySerializer {
    public:
        EntitySerializer() = default;
        ~EntitySerializer() noexcept = default;


        bool serialize(YAML::Emitter& out, robot2D::ecs::Entity& entity);
        bool deserialize(void* inputData, robot2D::ecs::Entity& entity,
                         bool& addToScene, std::vector<ChildInfo>& children);
    private:
    };

}