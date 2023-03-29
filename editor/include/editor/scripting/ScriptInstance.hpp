#pragma once
#include <memory>

#include <robot2D/Ecs/Entity.hpp>
#include "MonoClassWrapper.hpp"

extern "C" {
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClassField MonoClassField;
}

namespace editor {

    class ScriptEngineData;
    class ScriptInstance {
    public:
        using Ptr = std::shared_ptr<ScriptInstance>;
    public:
        ScriptInstance(ScriptEngineData* data, MonoClassWrapper::Ptr, robot2D::ecs::Entity);
        ~ScriptInstance() = default;

        MonoClassWrapper::Ptr getClassWrapper() { return m_class; }

        template<typename T>
        T getFieldValue(const std::string& name)
        {
            static_assert(sizeof(T) <= 16, "Type too large!");

            bool success = getFieldValueInternal(name, s_FieldValueBuffer);
            if (!success)
                return T();

            return *(T*)s_FieldValueBuffer;
        }

        template<typename T>
        void setFieldValue(const std::string& name, T value)
        {
            static_assert(sizeof(T) <= 16, "Type too large!");

            setFieldValueInternal(name, &value);
        }

        bool getFieldValueInternal(const std::string& name, void* buffer);
        bool setFieldValueInternal(const std::string& name, const void* value);

        void onCreateInvoke();
        void onUpdateInvoke(float dt);
    private:
        MonoClassWrapper::Ptr m_class;
        robot2D::ecs::Entity m_entity;

        inline static char s_FieldValueBuffer[16];
    };
} // namespace editor