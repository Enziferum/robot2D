#include <editor/ScriptInteractor.hpp>

namespace editor {
    IScriptInteractor::~IScriptInteractor() = default;



    ScriptInteractor::ScriptInteractor(EditorInteractor::WeakPtr editorInteractor,
                                       ScriptingEngineService::Ptr scriptingEngine):
                                       m_editorInteractor{editorInteractor}, m_scriptingEngine{scriptingEngine} {}

    SceneEntity ScriptInteractor::getEntity(std::uint64_t entityID) {

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

        std::transform(classes.begin(), classes.end(), result.begin(),
                       [](const auto& elem) {
            return elem.first;
        });

        return result;
    }

} // namespace editor