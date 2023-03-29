#include <editor/scripting/ScriptEngineData.hpp>
#include <editor/scripting/MonoClassWrapper.hpp>

namespace editor {

    ScriptEngineData::ScriptEngineData() = default;
    ScriptEngineData::~ScriptEngineData() = default;

    bool ScriptEngineData::hasEntityClass(const std::string& className) const {
        return m_entityClasses.find(className) != m_entityClasses.end();
    }

    bool ScriptEngineData::hasEntityFields(ScriptEngineData::entityUUID uuid) const {
        return m_entityScriptFields.find(uuid) != m_entityScriptFields.end();
    }
} // namespace editor