#pragma once
#include <vector>
#include <memory>

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

    class IEntitySerializer {
    public:
        virtual ~IEntitySerializer() noexcept = 0;
    };

    class EntityYAMLSerializer final: public IEntitySerializer {
    public:
        EntityYAMLSerializer() = default;
        EntityYAMLSerializer(const EntityYAMLSerializer& other) = delete;
        EntityYAMLSerializer& operator=(const EntityYAMLSerializer& other) = delete;
        EntityYAMLSerializer(EntityYAMLSerializer&& other) = delete;
        EntityYAMLSerializer& operator=(EntityYAMLSerializer&& other) = delete;
        ~EntityYAMLSerializer() noexcept override  = default;


        bool serialize(YAML::Emitter& out, robot2D::ecs::Entity& entity);
        bool deserialize(const YAML::detail::iterator_value& iterator, robot2D::ecs::Entity& entity,
                         bool& addToScene, std::vector<ChildInfo>& children);
    };



    template<typename T, typename ... Args>
    std::unique_ptr<T> getSerializer(Args&& ... args) {
        static_assert(std::is_base_of_v<IEntitySerializer, T> && "Custom Serializer must be base of IEntitySerializer");
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

}