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
        Entity, Collision2D, Transform,
        Drawable, Text, Animator
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
        MonoClassWrapper(MonoObject* rawClass);

        ~MonoClassWrapper() = default;


        void Instantiate(ScriptEngineData* data);

        bool registerField(std::string&& name, ScriptField scriptField) {
            return m_registerFields.insert(std::make_pair(name, std::move(scriptField))).second;
        }

        const std::unordered_map<std::string, ScriptField>& getFields() const { return m_registerFields; }

        /// TODO(a.raag): Exception callback pass or ???
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


                if(m_registerMethods.find(name) != m_registerMethods.end())
                    mono_runtime_invoke(m_registerMethods[name], m_instance, storage, nullptr);
            }
            else {
                if(m_registerMethods.find(name) != m_registerMethods.end())
                    mono_runtime_invoke(m_registerMethods[name], m_instance, nullptr, nullptr);
            }
        }

        template<typename ... Args>
        void callMethodDirectly(MonoMethod* method, Args&& ...args) {
            if constexpr (sizeof...(Args) > 0) {
                void* storage[sizeof...(Args)];
                auto tuple = std::make_tuple(std::forward<Args>(args)...);
                constexpr std::size_t size = std::tuple_size_v<decltype(tuple)> - 1;
                util::over_tuple<size>::iterate(tuple, [&storage](std::size_t n, auto&& value) {
                    storage[n] = static_cast<void*>(&std::forward<decltype(value)>(value));
                });

                mono_runtime_invoke(method, m_instance, storage, nullptr);
            }
            else {
                mono_runtime_invoke(method, m_instance, nullptr, nullptr);
            }
        }

        std::unordered_map<std::string, MonoMethod*>& getRegisterMethods()  { return m_registerMethods; }
        MonoObject* getInstance() const { return m_instance; }

        bool hasMethod(const std::string& name) const { return m_registerMethods.find(name) != m_registerMethods.end(); }
        void registerMethod(std::string name, int argsCount);
    private:
        bool getFieldValue(const std::string& name, void* buffer);
        bool setFieldValue(const std::string& name, const void* value);
    private:
        MonoClass* m_class{nullptr};
        MonoObject* m_instance{nullptr};
        std::unordered_map<std::string, MonoMethod*> m_registerMethods;
        std::unordered_map<std::string, ScriptField> m_registerFields;

        friend class ScriptInstance;
        friend class ScriptEngine;
    };

}