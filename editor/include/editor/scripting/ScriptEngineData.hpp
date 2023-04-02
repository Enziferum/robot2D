#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <filewatch/FileWatch.hpp>
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


    struct ScriptFieldInstance
    {
        ScriptField Field;

        ScriptFieldInstance()
        {
            memset(m_Buffer, 0, sizeof(m_Buffer));
        }

        template<typename T>
        T getValue()
        {
            static_assert(sizeof(T) <= 16, "Type too large!");
            return *(T*)m_Buffer;
        }

        template<typename T>
        void setValue(T value)
        {
            static_assert(sizeof(T) <= 16, "Type too large!");
            memcpy(m_Buffer, &value, sizeof(T));
        }
    private:
        uint8_t m_Buffer[16];

        friend class ScriptEngine;
        friend class ScriptInstance;
    };

    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


    class ScriptEngineData {
    public:
        using entityUUID = int;

        ScriptEngineData();
        ~ScriptEngineData();

        bool hasEntityClass(const std::string& className) const;
        bool hasEntityFields(entityUUID uuid) const;
        bool isReloadPending() const {
            return m_assemblyReloadPending;
        }
        void setReloadPending(bool flag) { m_assemblyReloadPending = flag; }
        void resetFilewatcher() { m_fileWatcher.reset(); }
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

        std::unordered_map<entityUUID, ScriptInstance::Ptr> m_entityInstances;
        std::unordered_map<entityUUID, ScriptFieldMap> m_entityScriptFields;

        MonoClassWrapper::Ptr m_entityClass;
        std::unique_ptr<filewatch::FileWatch<std::string>> m_fileWatcher;

        std::filesystem::path m_coreAssemblyFilepath;
        std::filesystem::path m_appAssemblyFilepath;
        bool m_assemblyReloadPending = false;

        std::vector<std::string> m_entityClassesNames;
        Scene* sceneContext;
    };
} // namespace editor