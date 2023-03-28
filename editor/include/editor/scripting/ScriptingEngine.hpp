#pragma once
#include <string>
#include <memory>
#include <filesystem>

#include <robot2D/Ecs/Entity.hpp>

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

    class Scene;
    class ScriptEngine {
    public:
        static void Init();
        static void Shutdown();
        static void reloadEngine();

        static Scene* getSceneContext();
        static void onCreateEntity(robot2D::ecs::Entity);
        static void onUpdateEntity(robot2D::ecs::Entity, float delta);


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
