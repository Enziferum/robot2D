#pragma once
#include <string>
#include <unordered_map>
#include <robot2D/Core/Assert.hpp>

namespace editor {
    struct ScriptField
    {
        ScriptFieldType Type;
        std::string Name;
        MonoClassField* ClassField;
    };


    class class_formatter {
    public:
        /// if class has class_id get class_id else return undefined class
        template<typename T>
        static std::string id() {}
    };


    //// Steps:
    /// 1. Refactor ScriptingEngine
    /// 2. Actualize problems
    /// 3. Fix
    /// 4. Android buildable ?

    template<std::size_t Size>
    struct ScriptFieldInstance
    {
        ScriptField Field;

        ScriptFieldInstance()
        {
            memset(m_Buffer, 0, sizeof(m_Buffer));
        }
        ScriptFieldInstance(const ScriptFieldInstance& other) = default;
        ScriptFieldInstance& operator=(const ScriptFieldInstance& other) = default;
        ScriptFieldInstance(ScriptFieldInstance&& other) = default;
        ScriptFieldInstance& operator=(ScriptFieldInstance&& other) = default;
        ~ScriptFieldInstance() = default;

        template<typename T>
        T getValue()
        {
            static_assert(sizeof(T) <= Size, "Type too large!");
            T* valueBuffer = (T*)m_Buffer;
            RB_CORE_ASSERT(valueBuffer, "Invalid ScriptFieldInstance");
            return *valueBuffer;
        }

        template<typename T>
        void setValue(T value)
        {
            static_assert(sizeof(T) <= Size, "Type too large!");
            memcpy(m_Buffer, &value, sizeof(T));
        }
    private:
        uint8_t m_Buffer[Size];

        friend class ScriptEngine;
        friend class ScriptInstance;
    };

    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


} // namespace editor