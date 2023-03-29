#include <mono/metadata/object.h>
#include <editor/scripting/ScriptInstance.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

namespace editor {

    ScriptInstance::ScriptInstance(ScriptEngineData* data, MonoClassWrapper::Ptr classWrapper,
                                   robot2D::ecs::Entity entity):
            m_class{std::move(classWrapper)} {
        int index = entity.getIndex();
        data -> m_entityClass -> callMethod(".ctor", index);
    }

    void ScriptInstance::onCreateInvoke() {
        m_class -> callMethod("OnCreate");
    }

    void ScriptInstance::onUpdateInvoke(float dt) {
        m_class -> callMethod("OnUpdate", dt);
    }

    bool ScriptInstance::getFieldValueInternal(const std::string& name, void* buffer) {
        return m_class -> getFieldValue(name, buffer);
    }

    bool ScriptInstance::setFieldValueInternal(const std::string& name, const void* value) {
        return m_class -> setFieldValue(name, value);
    }
} // namespace editor