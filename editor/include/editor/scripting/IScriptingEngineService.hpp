#pragma once
#include <memory>
#include <editor/SceneEntity.hpp>

namespace editor {
    class IScriptInteractor;
    class IScriptingEngineService {
    public:
        virtual ~IScriptingEngineService() = 0;

        virtual void onCreateEntity(SceneEntity sceneEntity) = 0;
        void setScriptInteractor();
        std::shared_ptr<IScriptInteractor> getInteractor() const;
    protected:
        std::shared_ptr<IScriptInteractor> m_interactor;
    };

} // namespace editor