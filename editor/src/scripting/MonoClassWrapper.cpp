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

    bool MonoClassWrapper::getFieldValue(const std::string &name, void *buffer) {
        const auto& fields = m_registerFields;
        auto it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        mono_field_get_value(m_instance, field.ClassField, buffer);
        return true;
    }

    bool MonoClassWrapper::setFieldValue(const std::string &name, const void *value) {
        const auto& fields = m_registerFields;
        auto it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        mono_field_set_value(m_instance, field.ClassField, (void*)value);
        return true;
    }
} // namespace editor