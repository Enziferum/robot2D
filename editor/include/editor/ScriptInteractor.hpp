#pragma once
#include <memory>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {

    class ScriptInteractor {
    public:
        using Ptr = ScriptInteractor*;

        virtual ~ScriptInteractor() = 0;
        virtual robot2D::ecs::Entity getByUUID(std::uint64_t entityID) = 0;

        virtual bool loadSceneRuntime(std::string&& name) = 0;
        virtual void loadSceneAsyncRuntime(std::string&& name) = 0;
        virtual void exitEngineRuntime() = 0;
    };

} // namespace editor