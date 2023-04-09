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

#include <mono/metadata/object.h>
#include <editor/scripting/ScriptInstance.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

namespace editor {

    ScriptInstance::ScriptInstance(ScriptEngineData* data,
                                   MonoClassWrapper::Ptr classWrapper,
                                   robot2D::ecs::Entity entity):
            m_class{std::move(classWrapper)},
            m_entity(std::move(entity)) {
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