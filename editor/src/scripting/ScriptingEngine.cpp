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

#include <cassert>
#include <string>
#include <fstream>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include <robot2D/util/Logger.hpp>
#include <robot2D/Core/Assert.hpp>

#include <mono/jit/jit.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>


#include <editor/Components.hpp>
#include <editor/Scene.hpp>
#include <editor/TaskQueue.hpp>

#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/scripting/ScriptingGlue.hpp>
#include <editor/scripting/ScriptInstance.hpp>
#include <editor/scripting/MonoClassWrapper.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

#include "mono_build_config.h"

namespace editor {
    #define MONO_FIELD_ATTRIBUTE_PRIVATE 1
    #define MONO_FIELD_ATTRIBUTE_PUBLIC 6

    static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
    {
        { "System.Single", ScriptFieldType::Float },
        { "System.Double", ScriptFieldType::Double },
        { "System.Boolean", ScriptFieldType::Bool },
        { "System.Char", ScriptFieldType::Char },
        { "System.Int16", ScriptFieldType::Short },
        { "System.Int32", ScriptFieldType::Int },
        { "System.Int64", ScriptFieldType::Long },
        { "System.Byte", ScriptFieldType::Byte },
        { "System.UInt16", ScriptFieldType::UShort },
        { "System.UInt32", ScriptFieldType::UInt },
        { "System.UInt64", ScriptFieldType::ULong },
        { "robot2D.Vector2", ScriptFieldType::Vector2 },
        { "robot2D.Vector3", ScriptFieldType::Vector3 },
        { "robot2D.Vector4", ScriptFieldType::Vector4 },
        { "robot2D.Entity", ScriptFieldType::Entity },
        { "robot2D.Collision2D", ScriptFieldType::Collision2D },
        { "robot2D.TransformComponent", ScriptFieldType::Transform },
    };

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

        ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
        {
            std::string typeName = mono_type_get_name(monoType);

            auto it = s_ScriptFieldTypeMap.find(typeName);
            if (it == s_ScriptFieldTypeMap.end())
            {
                RB_EDITOR_ERROR("Unknown type: {}", typeName);
                return ScriptFieldType::None;
            }

            return it->second;
        }

        const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
        {
            switch (fieldType)
            {
                case ScriptFieldType::None:    return "None";
                case ScriptFieldType::Float:   return "Float";
                case ScriptFieldType::Double:  return "Double";
                case ScriptFieldType::Bool:    return "Bool";
                case ScriptFieldType::Char:    return "Char";
                case ScriptFieldType::Byte:    return "Byte";
                case ScriptFieldType::Short:   return "Short";
                case ScriptFieldType::Int:     return "Int";
                case ScriptFieldType::Long:    return "Long";
                case ScriptFieldType::UByte:   return "UByte";
                case ScriptFieldType::UShort:  return "UShort";
                case ScriptFieldType::UInt:    return "UInt";
                case ScriptFieldType::ULong:   return "ULong";
                case ScriptFieldType::Vector2: return "Vector2";
                case ScriptFieldType::Vector3: return "Vector3";
                case ScriptFieldType::Vector4: return "Vector4";
                case ScriptFieldType::Entity:  return "Entity";
                case ScriptFieldType::Collision2D:  return "Collision2D";
                case ScriptFieldType::Transform:  return "Transform";
            }
            RB_CORE_ASSERT(false, "Unknown ScriptFieldType");
            return "None";
        }

        ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
        {
            if (fieldType == "None")    return ScriptFieldType::None;
            if (fieldType == "Float")   return ScriptFieldType::Float;
            if (fieldType == "Double")  return ScriptFieldType::Double;
            if (fieldType == "Bool")    return ScriptFieldType::Bool;
            if (fieldType == "Char")    return ScriptFieldType::Char;
            if (fieldType == "Byte")    return ScriptFieldType::Byte;
            if (fieldType == "Short")   return ScriptFieldType::Short;
            if (fieldType == "Int")     return ScriptFieldType::Int;
            if (fieldType == "Long")    return ScriptFieldType::Long;
            if (fieldType == "UByte")   return ScriptFieldType::UByte;
            if (fieldType == "UShort")  return ScriptFieldType::UShort;
            if (fieldType == "UInt")    return ScriptFieldType::UInt;
            if (fieldType == "ULong")   return ScriptFieldType::ULong;
            if (fieldType == "Vector2") return ScriptFieldType::Vector2;
            if (fieldType == "Vector3") return ScriptFieldType::Vector3;
            if (fieldType == "Vector4") return ScriptFieldType::Vector4;
            if (fieldType == "Entity")  return ScriptFieldType::Entity;
            if (fieldType == "Collision2D")  return ScriptFieldType::Collision2D;
            if (fieldType == "Transform")  return ScriptFieldType::Transform;

            RB_CORE_ASSERT(false, "Unknown ScriptFieldType");
            return ScriptFieldType::None;
        }

    }

    static ScriptEngineData* s_Data;

    struct MethodSignature {
        MethodSignature() = default;
        MethodSignature(const std::initializer_list<ScriptFieldType>&  types) {
            m_types.assign(types.begin(), types.end());
        }
        std::vector<ScriptFieldType> m_types;
    };

    std::unordered_map<std::string, MethodSignature> engineRegisteredMethods = {
            { "onCollision2DEnter", { { ScriptFieldType::Collision2D } } },
            { "onCollision2DExit", { { ScriptFieldType::Collision2D } } },
    };

    class ScriptEngineReloadTask: public ITask {
    public:
        explicit ScriptEngineReloadTask(ITaskFunction::Ptr func): ITask(func) {}
        ~ScriptEngineReloadTask() override = default;
        void execute() override {
            RB_EDITOR_INFO("Reloading Script Engine");
        }
    };

    static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
    {
        if (!s_Data -> isReloadPending() && change_type == filewatch::Event::modified)
        {
            s_Data -> setReloadPending(true);

            TaskQueue::GetQueue() -> addAsyncTask<ScriptEngineReloadTask>([](const ScriptEngineReloadTask& task) {
                s_Data -> resetFilewatcher() ;
                ScriptEngine::ReloadEngine();
            });
        }


    }



    void ScriptEngine::Init() {
        s_Data = new ScriptEngineData();

        InitMono();
        ScriptGlue::registerFunctions();

        bool status = loadCoreAssembly("res/script/robot2D_ScriptCore.dll");
        if(!status) {
            RB_EDITOR_ERROR("Can't load Core Scripting Engine Library");
            return;
        }

    }

    void ScriptEngine::InitAppRuntime(const fs::path& filePath) {
        bool status = loadAppAssembly(filePath.string());
        if(!status) {
            RB_EDITOR_ERROR("ScriptingEngine: Can't load Application's Script Library");
            return;
        }

        LoadAssemblyClasses();
        ScriptGlue::registerComponents();
        s_Data -> m_entityClass = std::make_shared<MonoClassWrapper>(s_Data,
                                                                     "robot2D", "Entity", true);
        s_Data -> m_entityClass -> registerMethod(".ctor", 1);
        s_Data -> m_entityClass -> registerMethod("setComponentField", 2);
        s_Data -> m_entityClass -> registerMethod("onCollision2DInternal", 3);

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
            if (!monoClass || monoClass == entityClass || mono_class_is_enum(monoClass))
                continue;

            bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
            if (!isEntity)
                continue;

            /// search
            s_Data -> m_entityClassesNames.emplace_back(fullName);

            auto scriptClass = std::make_shared<MonoClassWrapper>(s_Data, nameSpace, className);
            s_Data -> m_entityClasses[fullName] = scriptClass;

            int fieldCount = mono_class_num_fields(monoClass);
            void* iterator =  nullptr;

            while(MonoClassField* field = mono_class_get_fields(monoClass, &iterator)) {
                const char* fieldName = mono_field_get_name(field);
                uint32_t flags = mono_field_get_flags(field);

                if (flags & MONO_FIELD_ATTRIBUTE_PUBLIC)
                {
                    MonoType* type = mono_field_get_type(field);
                    ScriptFieldType fieldType = util::MonoTypeToScriptFieldType(type);
                    RB_EDITOR_WARN("ScriptingEngine: Field Type {0}", std::string( util::ScriptFieldTypeToString(fieldType)));

                    if(!scriptClass -> registerField(fieldName, { fieldType, fieldName, field })) {
                        RB_EDITOR_ERROR("ScriptingEngine: Can't register field type {0}",  std::string( util::ScriptFieldTypeToString(fieldType)));
                    }
                }
            }

            void* methodIterator = nullptr;
            while(MonoMethod* method = mono_class_get_methods(monoClass, &methodIterator)) {
                std::string methodName = mono_method_get_name(method);
                uint32_t methodsFlags;
                uint32_t flags = mono_method_get_flags(method, &methodsFlags);


                if(engineRegisteredMethods.find(methodName) != engineRegisteredMethods.end()) {
                    const auto& signature = engineRegisteredMethods[methodName];
                    RB_EDITOR_WARN("ScriptingEngine: Found Method: {0}", methodName);
                    auto methodToken = mono_method_get_token(method);
                    auto sig = mono_method_get_signature(method, s_Data -> m_appAssemblyImage, methodToken);

                    void* methodSigIter = nullptr;
                    int index = 0;
                    bool validSignature = true;

                    auto paramCount = mono_signature_get_param_count(sig);
                    while(MonoType* type = mono_signature_get_params(sig, &methodSigIter)) {
                        if(signature.m_types[index] != util::MonoTypeToScriptFieldType(type)) {
                            RB_EDITOR_ERROR("ScriptingEngine: Bad Signature for method {0}", methodName);
                            validSignature = false;
                        }
                        ++index;
                    }
                    if(validSignature)
                        scriptClass -> registerMethod(methodName, paramCount);
                }

            }
        }
    }

    bool ScriptEngine::loadCoreAssembly(const fs::path& filePath) {
        s_Data -> m_coreAssemblyFilepath = filePath;
        s_Data -> m_appDomain = mono_domain_create_appdomain("Robot2DScriptRuntime", nullptr);
        s_Data -> m_coreAssebly = util::loadMonoAssembly(filePath.string());
        if(s_Data -> m_coreAssebly == nullptr) {
            return false;
        }

        s_Data -> m_coreAssemblyImage = mono_assembly_get_image(s_Data -> m_coreAssebly);

        return true;
    }

    bool ScriptEngine::loadAppAssembly(const fs::path& filePath) {
        s_Data -> m_appAssemblyFilepath = filePath;
        s_Data -> m_appAssebly = util::loadMonoAssembly(filePath.string());
        if(s_Data -> m_appAssebly == nullptr) {
            return false;
        }

        s_Data -> m_appAssemblyImage = mono_assembly_get_image(s_Data -> m_appAssebly);
       // s_Data -> m_fileWatcher = std::make_unique<filewatch::FileWatch<std::string>>(filePath.string(),
        //            OnAppAssemblyFileSystemEvent);
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
            auto uuid = entity.getComponent<IDComponent>().ID;
            auto instance = std::make_shared<ScriptInstance>(s_Data,
                                                             s_Data -> m_entityClasses[sc.name], entity);
            s_Data -> m_entityInstances[uuid] = instance;


            if(s_Data -> hasEntityFields(uuid)) {
                const auto& fields = s_Data -> m_entityScriptFields.at(uuid);
                for(const auto& [name, field] : fields) {
                    /// TODO(a.raag): make part of Instance call
                    if(field.Field.Type == ScriptFieldType::Transform) {
                        auto klass = instance -> getClassWrapper();
                        auto regMethods = s_Data -> m_entityClass -> getRegisterMethods();
                        if(regMethods.find("setComponentField") != regMethods.end()) {
                            auto MonoString = mono_string_new(s_Data->m_appDomain, field.Field.Name.c_str());
                            void* storage[2] = { (void*)(MonoString), (void*)(field.m_Buffer)};
                            mono_runtime_invoke(regMethods["setComponentField"],
                                                klass -> getInstance(), storage, nullptr);
                        }
                    }
                    else
                        instance -> setFieldValueInternal(name, field.m_Buffer);
                }
            }

            instance -> onCreateInvoke();
        }

    }

    void ScriptEngine::onUpdateEntity(robot2D::ecs::Entity entity, float delta) {
        auto uuid = entity.getComponent<IDComponent>().ID;
        if(s_Data -> m_entityInstances.find(uuid) != s_Data -> m_entityInstances.end()) {
            auto instance = s_Data -> m_entityInstances[uuid];
            instance -> onUpdateInvoke(delta);
        }
        else {
            /// error
        }
    }

    void ScriptEngine::onRuntimeStart(ScriptInteractor::Ptr interactor) {
        s_Data -> interactor = interactor;
    }

    void ScriptEngine::onRuntimeStop() {
        s_Data -> interactor = nullptr;
        s_Data -> m_entityInstances.clear();
    }

    void ScriptEngine::onCollision2DBegin(const Physics2DContact& contact) {
        for(auto& [uuid, klass]: s_Data -> m_entityClasses) {
            if( klass -> hasMethod("onCollision2DEnter") ) {
                auto regMethods = s_Data -> m_entityClass -> getRegisterMethods();
                if(regMethods.find("onCollision2DInternal") != regMethods.end()) {
                    void* storage[3] = { (void*)&contact.entityA, (void*)&contact.entityB, (void*)&contact.contanctType};
                    mono_runtime_invoke(regMethods["onCollision2DInternal"],
                                        klass -> getInstance(), storage, nullptr);
                }
            }
        }
    }

    void ScriptEngine::onCollision2DEnd(const Physics2DContact& contact) {
        for(auto& [uuid, klass]: s_Data -> m_entityClasses) {
            if( klass -> hasMethod("onCollision2DExit")) {
                auto regMethods = s_Data -> m_entityClass -> getRegisterMethods();
                if(regMethods.find("onCollision2DInternal") != regMethods.end()) {
                    void* storage[3] = { (void*)&contact.entityA, (void*)&contact.entityB, (void*)&contact.contanctType};
                    mono_runtime_invoke(regMethods["onCollision2DInternal"],
                                        klass -> getInstance(), storage, nullptr);
                }
            }
        }
    }

    MonoImage* ScriptEngine::GetCoreAssemblyImage() {
        return s_Data -> m_coreAssemblyImage;
    }

    bool ScriptEngine::hasEntityClass(const std::string& name) {
        return s_Data -> hasEntityClass(name);
    }


    ScriptInteractor::Ptr ScriptEngine::getInteractor() {
        return s_Data -> interactor;
    }

    MonoClassWrapper::Ptr ScriptEngine::getEntityClass(const std::string& name) {
        return s_Data -> m_entityClasses.at(name);
    }

    ScriptFieldMap& ScriptEngine::getScriptFieldMap(robot2D::ecs::Entity entity) {
        auto uuid = entity.getComponent<IDComponent>().ID;
        auto& fields = s_Data -> m_entityScriptFields;
        return s_Data -> m_entityScriptFields[uuid];
    }

    ScriptInstance::Ptr ScriptEngine::getEntityScriptInstance(UUID entityID) {
        auto found = s_Data -> m_entityInstances.find(entityID);
        if(found == s_Data -> m_entityInstances.end())
            return nullptr;
        return found -> second;
    }

    void ScriptEngine::ReloadEngine() {
        mono_domain_set(mono_get_root_domain(), false);

        mono_domain_unload(s_Data -> m_appDomain);

        loadCoreAssembly(s_Data -> m_coreAssemblyFilepath);
        loadAppAssembly(s_Data -> m_appAssemblyFilepath);
        LoadAssemblyClasses();

        ScriptGlue::registerComponents();

        // Retrieve and instantiate class
        s_Data -> m_entityClass.reset();
        s_Data -> m_entityClass = std::make_shared<MonoClassWrapper>(s_Data,
                                                                     "robot2D", "Entity", true);
    }

    MonoClassWrapper::Ptr ScriptEngine::getManagedObject(UUID entityId) {
        return s_Data -> m_entityInstances[entityId] -> getClassWrapper();
    }

    const std::unordered_map<std::string, MonoClassWrapper::Ptr>& ScriptEngine::getClasses() {
        return s_Data -> getClasses();
    }


} // namespace editor
