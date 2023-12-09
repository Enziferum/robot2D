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
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <filewatch/FileWatch.hpp>
#include "editor/ScriptInteractor.hpp"
#include <editor/Uuid.hpp>
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
            static_assert(sizeof(T) <= 64, "Type too large!");
            return *(T*)m_Buffer;
        }

        template<typename T>
        void setValue(T value)
        {
            static_assert(sizeof(T) <= 64, "Type too large!");
            memcpy(m_Buffer, &value, sizeof(T));
        }
    private:
        uint8_t m_Buffer[64];

        friend class ScriptEngine;
        friend class ScriptInstance;
    };

    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


    class ScriptEngineData {
    public:
        ScriptEngineData();
        ~ScriptEngineData();

        bool hasEntityClass(const std::string& className) const;
        bool hasEntityFields(UUID uuid) const;
        bool isReloadPending() const {
            return m_assemblyReloadPending;
        }
        void setReloadPending(bool flag) { m_assemblyReloadPending = flag; }
        void resetFilewatcher() { m_fileWatcher.reset(); }
        const std::unordered_map<std::string, MonoClassWrapper::Ptr>& getClasses() const {
            return m_entityClasses;
        }
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

        std::unordered_map<UUID, ScriptInstance::Ptr> m_entityInstances;
        std::unordered_map<UUID, ScriptFieldMap> m_entityScriptFields;

        MonoClassWrapper::Ptr m_entityClass;
        std::unique_ptr<filewatch::FileWatch<std::string>> m_fileWatcher;

        std::filesystem::path m_coreAssemblyFilepath;
        std::filesystem::path m_appAssemblyFilepath;
        bool m_assemblyReloadPending = false;

        std::vector<std::string> m_entityClassesNames;
        ScriptInteractor::Ptr interactor;
    };
} // namespace editor