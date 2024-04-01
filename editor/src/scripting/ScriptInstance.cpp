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
#include <string>
#include <unordered_map>

#include <editor/Components.hpp>
#include <editor/scripting/ScriptInstance.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

namespace editor {

    namespace {
        enum class MethodType {
            Ctor,
            Create,
            Update
        };

        std::unordered_map<MethodType, std::string> g_methodNames = {
                { MethodType::Ctor, ".ctor" },
                { MethodType::Create, "OnCreate" },
                { MethodType::Update, "OnUpdate" },
        };
    }

    ScriptInstance::ScriptInstance(ScriptEngineData* data,
                                   MonoClassWrapper::Ptr classWrapper,
                                   SceneEntity entity):
            m_class{std::move(classWrapper)},
            m_entity(std::move(entity)) {
        m_class -> Instantiate(data);
        auto uuid = m_entity.getComponent<IDComponent>().ID;
        auto regMethods = data -> m_entityClass -> getRegisterMethods();
        if(regMethods.find(g_methodNames[MethodType::Ctor]) != regMethods.end())
            m_class -> callMethodDirectly(regMethods[g_methodNames[MethodType::Ctor]], uuid);
    }

    void ScriptInstance::onCreateInvoke() {
        m_class -> callMethod(g_methodNames[MethodType::Create]);
    }

    void ScriptInstance::onUpdateInvoke(float dt) {
        m_class -> callMethod(g_methodNames[MethodType::Update], dt);
    }

    bool ScriptInstance::getFieldValueInternal(const std::string& name, void* buffer) {
        return m_class -> getFieldValue(name, buffer);
    }

    bool ScriptInstance::setFieldValueInternal(const std::string& name, const void* value) {
        return m_class -> setFieldValue(name, value);
    }

} // namespace editor