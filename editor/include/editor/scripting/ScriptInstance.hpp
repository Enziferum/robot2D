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

        void onCreateInvoke();
        void onUpdateInvoke(float dt);
    private:
        MonoClassWrapper::Ptr m_class;
        robot2D::ecs::Entity m_entity;
    };
} // namespace editor