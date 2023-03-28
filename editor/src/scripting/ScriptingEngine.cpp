#include <cassert>
#include <string>
#include <fstream>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include <mono/jit/jit.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <editor/Components.hpp>
#include <editor/Scene.hpp>

#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/scripting/ScriptingGlue.hpp>
#include <editor/scripting/ScriptInstance.hpp>
#include <editor/scripting/MonoClassWrapper.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

#include "mono_build_config.h"

namespace editor {

    namespace util {
        char* ReadBytes(const std::string& filepath, uint32_t* outSize)
        {
            std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

            if (!stream)
            {
                // Failed to open the file
                return nullptr;
            }

            std::streampos end = stream.tellg();
            stream.seekg(0, std::ios::beg);
            uint32_t size = end - stream.tellg();

            if (size == 0)
            {
                // File is empty
                return nullptr;
            }

            char* buffer = new char[size];
            stream.read((char*)buffer, size);
            stream.close();

            *outSize = size;
            return buffer;
        }

        MonoAssembly* loadMonoAssembly(const std::string& assemblyPath)
        {
            uint32_t fileSize = 0;
            char* fileData = ReadBytes(assemblyPath, &fileSize);

            // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
            MonoImageOpenStatus status;
            MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

            if (status != MONO_IMAGE_OK)
            {
                const char* errorMessage = mono_image_strerror(status);
                // Log some error message using the errorMessage data
                return nullptr;
            }

            bool loadPdb = true;
            if(loadPdb) {
                std::filesystem::path pdbPath = assemblyPath;
                pdbPath.replace_extension(".mdb");
                if(std::filesystem::exists(pdbPath)) {
                    uint32_t pdbFileSize = 0;
                    char* pdbFileData = ReadBytes(pdbPath.string(), &pdbFileSize);
                    mono_debug_open_image_from_memory(image, (const mono_byte*)pdbFileData, pdbFileSize);
                    delete [] pdbFileData;
                }
            }

            MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
            mono_image_close(image);

            // Don't forget to free the file data
            delete[] fileData;

            return assembly;
        }

        void PrintAssemblyTypes(MonoAssembly* assembly)
        {
            MonoImage* image = mono_assembly_get_image(assembly);
            const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
            int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

            for (int32_t i = 0; i < numTypes; i++)
            {
                uint32_t cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

                const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
                const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

                printf("%s.%s\n", nameSpace, name);
            }
        }

    }

    static ScriptEngineData* s_Data;


    void ScriptEngine::Init() {
        s_Data = new ScriptEngineData();

        InitMono();
        ScriptGlue::registerFunctions();

        bool status = loadCoreAssembly("res/script/robot2D_ScriptCore.dll");
        if(!status) {
            ///
            return;
        }

        // TODO(a.raag): get from actual project //
        std::string scriptModulePath = "C:\\Users\\User\\Documents\\dev\\robot2DProjects\\ScriptDemo\\assets\\scripts\\bin\\ScriptDemo.dll";
        status = loadAppAssembly(scriptModulePath);
        if(!status) {
            ///
            return;
        }

        LoadAssemblyClasses();
        ScriptGlue::registerComponents();
        util::PrintAssemblyTypes(s_Data -> m_coreAssebly);
        util::PrintAssemblyTypes(s_Data -> m_appAssebly);
    }

    void ScriptEngine::LoadAssemblyClasses()
    {
        s_Data -> m_entityClasses.clear();

        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data ->
                m_appAssemblyImage, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        MonoClass* entityClass = mono_class_from_name(s_Data -> m_coreAssemblyImage, "robot2D", "Entity");
        s_Data -> m_entityClass = std::make_shared<MonoClassWrapper>(s_Data, "robot2D", "Entity", true);

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(s_Data -> m_appAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* className = mono_metadata_string_heap(s_Data -> m_appAssemblyImage, cols[MONO_TYPEDEF_NAME]);
            std::string fullName;
            if (strlen(nameSpace) != 0)
                fullName = fmt::format("{}.{}", nameSpace, className);
            else
                fullName = className;

            MonoClass* monoClass = mono_class_from_name(s_Data -> m_appAssemblyImage, nameSpace, className);

            if (monoClass == entityClass)
                continue;

            bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
            if (!isEntity)
                continue;

            /// search
            s_Data -> m_entityClassesNames.emplace_back(fullName);

            auto scriptClass = std::make_shared<MonoClassWrapper>(s_Data, nameSpace, className);
            s_Data -> m_entityClasses[fullName] = scriptClass;
        }
    }

    bool ScriptEngine::loadCoreAssembly(const fs::path& filePath) {
        s_Data -> m_appDomain = mono_domain_create_appdomain("Robot2DScriptRuntime", nullptr);
        s_Data -> m_coreAssebly = util::loadMonoAssembly(filePath.string());
        if(s_Data -> m_coreAssebly == nullptr) {
            return false;
        }

        s_Data -> m_coreAssemblyImage = mono_assembly_get_image(s_Data -> m_coreAssebly);

        return true;
    }

    bool ScriptEngine::loadAppAssembly(const fs::path& filePath) {
        s_Data -> m_appAssebly = util::loadMonoAssembly(filePath.string());
        if(s_Data -> m_appAssebly == nullptr) {
            return false;
        }

        s_Data -> m_appAssemblyImage = mono_assembly_get_image(s_Data -> m_appAssebly);

        return true;
    }

    void ScriptEngine::InitMono() {
        bool enableDebug = false;
        mono_set_dirs(INTERNAL_MONO_ASSEMBLY_DIR, INTERNAL_MONO_CONFIG_DIR);

        if(enableDebug) {
            const char* options[] =
                    {
                            "--soft-breakpoints",
                            "--debugger-agent=transport=dt_socket,suspend=n,server=y,address=127.0.0.1:55555"
                    };

            mono_jit_parse_options(sizeof(options) / sizeof(char*), const_cast<char**>(options));
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);

        }

        MonoDomain* rootDomain = mono_jit_init("Robot2DJitRuntime");
        assert(rootDomain != nullptr);
        s_Data -> m_rootDomain = rootDomain;

        if(enableDebug)
            mono_debug_domain_create(s_Data -> m_rootDomain);

        mono_thread_set_main(mono_thread_current());
    }

    void ScriptEngine::Shutdown() {
        ShutdownMono();
        delete s_Data;
    }


    void ScriptEngine::ShutdownMono() {
        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(s_Data -> m_appDomain);
        s_Data -> m_appDomain = nullptr;
        mono_jit_cleanup(s_Data -> m_rootDomain);
    }

    void ScriptEngine::onCreateEntity(robot2D::ecs::Entity entity) {
        const auto& sc = entity.getComponent<ScriptComponent>();
        if(hasEntityClass(sc.name)) {
            auto uuid = entity.getIndex(); // TODO(a.raag): make real uuid not index
            auto instance = std::make_shared<ScriptInstance>(s_Data, s_Data -> m_entityClasses[sc.name], entity);
            s_Data -> m_entityInstances[uuid] = instance;
            instance -> onCreateInvoke();
        }

    }

    void ScriptEngine::onUpdateEntity(robot2D::ecs::Entity entity, float delta) {
        auto uuid = entity.getIndex(); // TODO(a.raag): make real uuid not index
        if(s_Data -> m_entityInstances.find(uuid) != s_Data -> m_entityInstances.end()) {
            auto instance = s_Data -> m_entityInstances[uuid];
            instance -> onUpdateInvoke(delta);
        }
        else {
            /// error
        }
    }

    bool ScriptEngine::hasEntityClass(const std::string& name) {
        return s_Data -> hasEntityClass(name);
    }

    void ScriptEngine::onRuntimeStart(Scene* scene) {
        s_Data -> sceneContext = scene;
    }

    void ScriptEngine::onRuntimeStop() {
        s_Data -> sceneContext = nullptr;
        s_Data -> m_entityInstances.clear();
    }

    MonoImage* ScriptEngine::GetCoreAssemblyImage() {
        return s_Data -> m_coreAssemblyImage;
    }

    Scene* ScriptEngine::getSceneContext() {
        return s_Data -> sceneContext;
    }


} // namespace editor
