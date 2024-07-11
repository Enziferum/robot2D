#pragma once
#include <vector>
#include <memory>

#include <yaml-cpp/yaml.h>

#include <editor/SceneEntity.hpp>
#include <editor/Uuid.hpp>
#include <editor/scripting/ScriptingEngineService.hpp>

namespace editor {
    using ChildPair = std::pair<UUID, std::vector<UUID>>;

    struct ChildInfo {
        bool isChild{false};
        SceneEntity self;
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


        bool serialize(YAML::Emitter& out, const SceneEntity& entity, IScriptInteractorFrom::Ptr scriptingEngine);
        bool deserialize(const YAML::detail::iterator_value& iterator,
                         SceneEntity& entity,
                         bool& addToScene,
                         std::vector<ChildInfo>& children,
                         IScriptInteractorFrom::Ptr scriptInteractor);
    };



    template<typename T, typename ... Args>
    std::unique_ptr<T> getSerializer(Args&& ... args) {
        static_assert(std::is_base_of_v<IEntitySerializer, T> && "Custom Serializer must be base of IEntitySerializer");
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

}