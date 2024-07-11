#pragma once
#include "editor/ScriptInteractor.hpp"

namespace editor {

    class IScriptingEngineService {
    public:
        virtual ~IScriptingEngineService() = 0;


        void setScriptInteractor();
        IScriptInteractor::Ptr getInteractor() const;
    protected:
        IScriptInteractor::Ptr m_interactor;
    };

} // namespace editor