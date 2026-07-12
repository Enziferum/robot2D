/*********************************************************************
(c) Alex Raag 2026
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

#include <editor/ScriptInteractor.hpp>
#include <editor/scripting/ScriptingEngineService.hpp>
#include <editor/EditorInteractor.hpp>

namespace editor {

    namespace util {
        FieldType convertFromScript(const ScriptFieldType& type) {
            switch(type) {
                default:
                    return FieldType::None;
                case ScriptFieldType::Float:
                    return FieldType::Float;
                case ScriptFieldType::Int:
                    return FieldType::Int;
                case ScriptFieldType::Transform:
                    return FieldType::Transform;
                case ScriptFieldType::Entity:
                    return FieldType::Entity;
            }
        }

        ScriptFieldType convert2Script(const FieldType& type) {
            switch(type) {
                default:
                    return ScriptFieldType::None;
                case FieldType::Float:
                    return ScriptFieldType::Float;
                case FieldType::Int:
                    return ScriptFieldType::Int;
                case FieldType::Transform:
                    return ScriptFieldType::Transform;
                case FieldType::Entity:
                    return ScriptFieldType::Entity;
            }
        }
    }

    IScriptInteractorFrom::~IScriptInteractorFrom() = default;
    IScriptInteractor::~IScriptInteractor() = default;



    ScriptInteractor::ScriptInteractor(EditorInteractor::WeakPtr editorInteractor,
                                       ScriptingEngineService* scriptingEngine):
                                       m_editorInteractor{editorInteractor}, m_scriptingEngine{scriptingEngine} {}

    SceneEntity ScriptInteractor::getEntity(std::uint64_t entityID) {
        auto interactor = m_editorInteractor.lock();
        if(!interactor)
            return {};
        return interactor -> getEntity(UUID{entityID});
    }

    SceneEntity ScriptInteractor::getEntity(const std::string& name) {
        auto interactor = m_editorInteractor.lock();
        if(!interactor)
            return {};
        return interactor -> getEntity(name);
    }

    void ScriptInteractor::loadSceneAsyncRuntime(std::string&& name) {

    }

    bool ScriptInteractor::loadSceneRuntime(std::string &&name) {

    }

    void ScriptInteractor::exitEngineRuntime() {

    }

    SceneEntity ScriptInteractor::duplicateRuntime(editor::SceneEntity entity, robot2D::vec2f position) {

    }


    void ScriptInteractor::setScriptClass(std::string name, UUID uuid) {
        auto entityClass = m_scriptingEngine -> getEntityClass(name);
        if(!entityClass)
            return;
        const auto& registredFields = entityClass -> getFields();
        for(const auto& [name, regField]: registredFields) {
            auto& fieldMap = m_fields[uuid];
            auto& field = fieldMap[name];
            field.setType(util::convertFromScript(regField.Type));
            field.setName(regField.Name);
        }
    }


    bool ScriptInteractor::hasFields(UUID uuid) const {
        return (m_fields.find(uuid) != m_fields.end());
    }

    IScriptInteractorFrom::FieldMap& ScriptInteractor::getFields(UUID uuid) {
        return m_fields[uuid];
    }

    bool ScriptInteractor::hasEntityClass(const std::string& name) const {
        return m_scriptingEngine -> hasEntityClass(name);
    }

    std::vector<std::string> ScriptInteractor::getClassesNames() const {
        const auto& classes = m_scriptingEngine -> getClasses();
        std::vector<std::string> result;
        result.reserve(classes.size());

        /*std::transform(classes.cbegin(), classes.cend(), result.begin(),
                       [](const auto& elem) {
            return elem.first;
        });*/

        for(const auto& [name, wrapper]: classes) {
            result.push_back(name);
        }


        return result;
    }

    void ScriptInteractor::callScriptMethod(UUID uuid, const std::string& methodName) {
        auto instance = m_scriptingEngine -> getEntityScriptInstance(uuid);
        if(instance && instance -> getClassWrapper())
            instance -> getClassWrapper() -> callMethod(methodName);
    }

    void ScriptInteractor::setEditorCamera(EditorCamera::Ptr editorCamera) {
        m_scriptingEngine -> SetCamera(editorCamera);
    }

    ScriptingEngineService* ScriptInteractor::getScriptingEngine() const {
        return m_scriptingEngine;
    }

} // namespace editor