#pragma once
#include <memory>
#include <editor/SceneEntity.hpp>

namespace editor {

    class ScriptInteractor {
    public:
        using Ptr = ScriptInteractor*;

        virtual ~ScriptInteractor() = 0;
        virtual SceneEntity getByUUID(std::uint64_t entityID) = 0;

        virtual bool loadSceneRuntime(std::string&& name) = 0;
        virtual void loadSceneAsyncRuntime(std::string&& name) = 0;
        virtual void exitEngineRuntime() = 0;
        virtual SceneEntity duplicateRuntime(SceneEntity entity,
                                                      robot2D::vec2f position = robot2D::vec2f{}) = 0;
    };

} // namespace editor