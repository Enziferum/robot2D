/*********************************************************************
(c) Alex Raag 2023
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <robot2D/Util/Logger.hpp>

#include <editor/scripting/MonoClassWrapper.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

namespace editor {

    MonoClassWrapper::MonoClassWrapper(ScriptEngineData* data, std::string namespaceStr, std::string className, bool isCore ) {

            m_class = mono_class_from_name(isCore ? data -> m_coreAssemblyImage : data -> m_appAssemblyImage,
                                           namespaceStr.c_str(), className.c_str());
            if(!m_class) {
                RB_EDITOR_ERROR("MonoClassWrapper: Can't create mono class object");
                /// TODO(a.raag): throw exception error
            }

    }

    MonoClassWrapper::MonoClassWrapper(MonoObject* rawClass) {
        m_class = mono_object_get_class(rawClass);
        if(!m_class) {
            RB_EDITOR_ERROR("MonoClassWrapper: Can't create mono class object");
            /// TODO(a.raag): throw exception error
        }
    }


    void MonoClassWrapper::Instantiate(ScriptEngineData* data) {
        m_instance = mono_object_new(data -> m_appDomain, m_class);
        if(!m_instance) {
            RB_EDITOR_ERROR("MonoClassWrapper: Can't create mono instance object");
            /// TODO(a.raag): throw exception error
        }
        mono_runtime_object_init(m_instance);
    }

    void MonoClassWrapper::registerMethod(std::string name, int argsCount) {
        MonoMethod* function = mono_class_get_method_from_name(m_class, name.c_str(), argsCount);
        if(!function) {
            //RB_EDITOR_WARN("MonoClassWrapper: Can't register method {0}", name);
            return;
        }
        m_registerMethods[name] = function;
    }

    bool MonoClassWrapper::getFieldValue(const std::string& name, void* buffer) {
        const auto& fields = m_registerFields;
        auto it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it -> second;
        mono_field_get_value(m_instance, field.ClassField, buffer);
        return true;
    }

    bool MonoClassWrapper::setFieldValue(const std::string& name, const void* value) {
        const auto& fields = m_registerFields;
        auto it = fields.find(name);
        if (it == fields.end())
            return false;

        const ScriptField& field = it -> second;
        mono_field_set_value(m_instance, field.ClassField, (void*)value);

        return true;
    }



} // namespace editor