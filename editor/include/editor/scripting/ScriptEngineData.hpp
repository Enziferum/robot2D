#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <editor/Scene.hpp>

#include "ScriptInstance.hpp"
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

    class ScriptEngineData {
    public:
        ScriptEngineData();
        ~ScriptEngineData();

        bool hasEntityClass(const std::string& className) const;
    private:
        friend class ScriptEngine;
        friend class MonoClassWrapper;
        friend class ScriptInstance;

        MonoDomain* m_rootDomain{nullptr};
        MonoDomain* m_appDomain{nullptr};
        MonoAssembly* m_coreAssebly{nullptr};
        MonoAssembly* m_appAssebly{nullptr};
        MonoImage* m_coreAssemblyImage{nullptr};
        MonoImage* m_appAssemblyImage{nullptr};

        std::unordered_map<std::string, MonoClassWrapper::Ptr> m_entityClasses;
        std::unordered_map<int, ScriptInstance::Ptr> m_entityInstances;
        MonoClassWrapper::Ptr m_entityClass;

        std::vector<std::string> m_entityClassesNames;
        Scene* sceneContext;
    };
} // namespace editor