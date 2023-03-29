#pragma once
#include <string>
#include <memory>
#include <filesystem>

#include <robot2D/Ecs/Entity.hpp>
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

    class Scene;
    class ScriptEngine {
    public:
        static void Init();
        static void Shutdown();
        static void reloadEngine();

        static Scene* getSceneContext();
        static void onCreateEntity(robot2D::ecs::Entity);
        static void onUpdateEntity(robot2D::ecs::Entity, float delta);

        static ScriptInstance::Ptr getEntityScriptInstance(robot2D::ecs::EntityID entityID);
        static MonoClassWrapper::Ptr getEntityClass(const std::string& name);
        static ScriptFieldMap& getScriptFieldMap(robot2D::ecs::Entity entity);

        static void onRuntimeStart(Scene* scene);
        static void onRuntimeStop();
        static MonoImage* GetCoreAssemblyImage();
        static bool hasEntityClass(const std::string& name);
    private:
        static bool loadCoreAssembly(const fs::path& );
        static bool loadAppAssembly(const fs::path& );
    private:
        static void InitMono();
        static void ShutdownMono();
        static void LoadAssemblyClasses();
    };
}
