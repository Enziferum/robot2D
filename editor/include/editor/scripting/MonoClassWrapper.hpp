#pragma once
#include <cassert>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include <mono/metadata/object.h>



extern "C" {
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
}

namespace editor {

    namespace util {
        template<unsigned N>
        struct over_tuple {
            template<typename Tuple, typename Func>
            static void iterate(Tuple&& tuple, Func&& func) {
                func(N, std::get<N>(std::forward<Tuple>(tuple)));
                over_tuple<N-1>::iterate(std::forward<Tuple>(tuple), std::forward<Func>(func));
            }
        };

        template<>
        struct over_tuple<0> {
            template<typename Tuple, typename Func>
            static void iterate(Tuple&& tuple, Func&& func) {
                func(0, std::get<0>(std::forward<Tuple>(tuple)));
            }
        };

    }


    enum class ScriptFieldType
    {
        None = 0,
        Float, Double,
        Bool, Char, Byte, Short, Int, Long,
        UByte, UShort, UInt, ULong,
        Vector2, Vector3, Vector4,
        Entity
    };


    struct ScriptField
    {
        ScriptFieldType Type;
        std::string Name;

        MonoClassField* ClassField;
    };

    class ScriptEngineData;
    class MonoClassWrapper {
    public:
        using Ptr = std::shared_ptr<MonoClassWrapper>;
    public:
        MonoClassWrapper(ScriptEngineData* data,
                         std::string namespaceStr, std::string className, bool isCore = false);
        ~MonoClassWrapper() = default;

        bool registerField(std::string&& name, ScriptField scriptField) {
            return m_registerFields.insert(std::make_pair(name, std::move(scriptField))).second;
        }

        const std::unordered_map<std::string, ScriptField>& getFields() const { return m_registerFields; }

        template<typename ... Args>
        void callMethod(std::string name, Args&& ...args) {
            if(m_registerMethods.find(name) == m_registerMethods.end())
                registerMethod(name, sizeof...(Args));

            if constexpr (sizeof...(Args) > 0) {
                void* storage[sizeof...(Args)];
                auto tuple = std::make_tuple(std::forward<Args>(args)...);
                constexpr std::size_t size = std::tuple_size_v<decltype(tuple)> - 1;
                util::over_tuple<size>::iterate(tuple, [&storage](std::size_t n, auto&& value) {
                    storage[n] = static_cast<void*>(&std::forward<decltype(value)>(value));
                });

                mono_runtime_invoke(m_registerMethods[name], m_instance, storage, nullptr);
            }
            else {
                mono_runtime_invoke(m_registerMethods[name], m_instance, nullptr, nullptr);
            }
        }
    private:
        void registerMethod(std::string name, int argsCount);
        bool getFieldValue(const std::string& name, void* buffer);
        bool setFieldValue(const std::string& name, const void* value);
    private:
        MonoClass* m_class{nullptr};
        MonoObject* m_instance{nullptr};
        std::unordered_map<std::string, MonoMethod*> m_registerMethods;
        std::unordered_map<std::string, ScriptField> m_registerFields;
        friend class ScriptInstance;
    };

}