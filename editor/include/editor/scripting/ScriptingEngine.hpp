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
#include <string>
#include <memory>
#include <filesystem>

#include <robot2D/Ecs/Entity.hpp>
#include "editor/ScriptInteractor.hpp"
#include "MonoClassWrapper.hpp"
#include "ScriptInstance.hpp"
#include "ScriptEngineData.hpp"

extern "C" {
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
    typedef struct _MonoAssembly MonoAssembly;
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoClassField MonoClassField;
}

namespace editor {
    namespace fs = std::filesystem;

    namespace util {
        const char* ScriptFieldTypeToString(ScriptFieldType fieldType);
        ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType);
    }

    class ScriptEngine {
    public:
        static void Init();
        static void InitAppRuntime(const fs::path& filePath);

        static void Shutdown();
        static void ReloadEngine();

        static ScriptInteractor::Ptr getInteractor();
        static void onCreateEntity(robot2D::ecs::Entity);
        static void onUpdateEntity(robot2D::ecs::Entity, float delta);

        static MonoClassWrapper::Ptr getManagedObject(robot2D::ecs::EntityID);

        static void onCollision2DBegin();
        static void onCollision2DEnd();

        static ScriptInstance::Ptr getEntityScriptInstance(robot2D::ecs::EntityID entityID);
        static MonoClassWrapper::Ptr getEntityClass(const std::string& name);
        static ScriptFieldMap& getScriptFieldMap(robot2D::ecs::Entity entity);

        static void onRuntimeStart(ScriptInteractor::Ptr interactor);
        static void onRuntimeStop();
        static MonoImage* GetCoreAssemblyImage();
        static bool hasEntityClass(const std::string& name);
        static const std::unordered_map<std::string, MonoClassWrapper::Ptr>& getClasses();
    private:
        static bool loadCoreAssembly(const fs::path& );
        static bool loadAppAssembly(const fs::path& );
    private:
        static void InitMono();
        static void ShutdownMono();
        static void LoadAssemblyClasses();
    };
}
