#include <editor/scripting/ScriptInstance.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

namespace editor {

    ScriptInstance::ScriptInstance(ScriptEngineData* data, MonoClassWrapper::Ptr classWrapper,
                                   robot2D::ecs::Entity entity):
            m_class{std::move(classWrapper)} {
        data -> m_entityClass -> callMethod(".ctor", entity.getIndex());
    }

    void ScriptInstance::onCreateInvoke() {
        m_class -> callMethod("OnCreate");
    }

    void ScriptInstance::onUpdateInvoke(float dt) {
        m_class -> callMethod("OnUpdate", dt);
    }
} // namespace editor