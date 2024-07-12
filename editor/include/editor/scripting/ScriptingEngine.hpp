/*********************************************************************
(c) Alex Raag 2024
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

#include <robot2D/Graphics/RenderWindow.hpp>

#include <editor/SceneEntity.hpp>
#include <editor/ScriptInteractor.hpp>
#include <editor/physics/Box2DPhysicsAdapter.hpp>

#include "MonoClassWrapper.hpp"
#include "ScriptInstance.hpp"
#include "ScriptEngineData.hpp"
#include "ScriptingGlue.hpp"
#include "ScriptingEngineService.hpp"
#include "ScriptingEngineInternalService.hpp"

namespace editor {
    namespace fs = std::filesystem;

    namespace util {
        const char* ScriptFieldTypeToString(ScriptFieldType fieldType);
        ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType);
    }

    class MonoImageWrapper {
    public:
        MonoImageWrapper();
        ~MonoImageWrapper() = default;


    private:
        MonoImage* image;
    };

    class ScriptEngine: public ScriptingEngineService, public ScriptingEngineInternalService {
    public:
        ScriptEngine() = default;
        ScriptEngine(const ScriptEngine& other) = delete;
        ScriptEngine& operator=(const ScriptEngine& other) = delete;
        ScriptEngine(ScriptEngine&& other) = delete;
        ScriptEngine& operator=(ScriptEngine&& other) = delete;
        ~ScriptEngine() override = default;

        //////////////////////////// Base Functions ///////////////////////
        void Init(const std::string& engineDLLPath);
        void runtimeInit(const std::string& filePath) override;

        void Shutdown();
        void ReloadEngine();

        void SetWindow(robot2D::RenderWindow* window);
        //////////////////////////// Base Functions ///////////////////////

        /////////////////////////// ScriptingEngineService ///////////////////
        void SetCamera(IEditorCamera::Ptr camera) override;
        void onCreateEntity(SceneEntity sceneEntity) override;
        void onUpdateEntity(SceneEntity sceneEntity, float delta) override;

        void onCollision2DBegin(const Physics2DContact& contact) override;
        void onCollision2DEnd(const Physics2DContact& contact) override;

        void onCollision2DBeginTrigger(const Physics2DContact& contact) override;
        void onCollision2DEndTrigger(const Physics2DContact& contact) override;

        void onRuntimeStart(IScriptInteractorFrom::Ptr interactor) override;
        void onRuntimeStop() override;

        ScriptInstance::Ptr getEntityScriptInstance(UUID entityID);
        MonoClassWrapper::Ptr getEntityClass(const std::string& name);
        ScriptFieldMap& getScriptFieldMap(SceneEntity entity);

        bool hasEntityClass(const std::string& name);
        const std::unordered_map<std::string, MonoClassWrapper::Ptr>& getClasses() const;
        /////////////////////////// ScriptingEngineService ///////////////////

        /////////////////// Scripting Glue ////////////////////////////////
        MonoImage* GetCoreAssemblyImage() const;
        MonoClassWrapper::Ptr getManagedObject(UUID uuid) const;

        IScriptInteractorFrom::Ptr getInteractor();
        robot2D::Window* GetWindow() const override;
        IEditorCamera::Ptr GetCamera() const override;
        /////////////////// Scripting Glue ////////////////////////////////
    private:
        ScriptInstance::Ptr CloneObject(MonoObject* cloneObject);
        bool loadCoreAssembly(const fs::path& );
        bool loadAppAssembly(const fs::path& );

        void InitMono();
        void ShutdownMono();
        void LoadAssemblyClasses();
    };
}
