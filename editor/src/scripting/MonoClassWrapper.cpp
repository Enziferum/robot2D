#include <editor/scripting/MonoClassWrapper.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

namespace editor {

    MonoClassWrapper::MonoClassWrapper(ScriptEngineData* data, std::string namespaceStr, std::string className, bool isCore ) {
           /// TODO(a.raag): check for errors ///
            m_class = mono_class_from_name(isCore ? data -> m_coreAssemblyImage : data -> m_appAssemblyImage, namespaceStr.c_str(), className.c_str());
            m_instance = mono_object_new(data -> m_appDomain, m_class);
            mono_runtime_object_init(m_instance);
    }

    void MonoClassWrapper::registerMethod(std::string name, int argsCount) {
        MonoMethod* function = mono_class_get_method_from_name(m_class, name.c_str(), argsCount);
        m_registerMethods[name] = function;
    }
} // namespace editor