#include <editor/scripting/IScriptingEngineService.hpp>
#include <editor/ScriptInteractor.hpp>

namespace editor {
    IScriptingEngineService::~IScriptingEngineService() = default;


    void IScriptingEngineService::setScriptInteractor(std::shared_ptr<IScriptInteractor> interactor) {
        m_interactor = interactor;
    }

    std::shared_ptr<IScriptInteractor> IScriptingEngineService::getInteractor() const {
        return m_interactor;
    }

} // namespace editor