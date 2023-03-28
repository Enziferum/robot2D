#include <functional>
#include <unordered_map>
#include <robot2D/Core/Keyboard.hpp>
#include <robot2D/Ecs/Entity.hpp>

#include <mono/metadata/loader.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <editor/scripting/ScriptingGlue.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/Scene.hpp>
#include <editor/Components.hpp>

namespace editor {

    #define RB_ADD_INTERNAL_CALL(Name) mono_add_internal_call("robot2D.InternalCalls::" #Name, (void*)Name)

    static std::unordered_map<MonoType*, std::function<bool(robot2D::ecs::Entity)>> s_EntityHasComponentFuncs;

    static void NativeLog(MonoString* string, int parameter)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        std::cout << str << ", " << parameter << std::endl;
    }

    static bool Entity_HasComponent(int entityID, MonoReflectionType* componentType)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        //HZ_CORE_ASSERT(scene);
        robot2D::ecs::Entity entity = scene -> getByIndex(entityID);
        //HZ_CORE_ASSERT(entity);

        MonoType* managedType = mono_reflection_type_get_type(componentType);
        //HZ_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
        bool hasComp = s_EntityHasComponentFuncs.at(managedType)(entity);
        return hasComp;
    }

    static void TransformComponent_GetTranslation(int entityID, robot2D::vec2f* outTranslation)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        robot2D::ecs::Entity entity = scene -> getByIndex(entityID);
        *outTranslation = entity.getComponent<TransformComponent>().getPosition();
    }

    static void TransformComponent_SetTranslation(int entityID, robot2D::vec2f* translation)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        robot2D::ecs::Entity entity = scene -> getByIndex(entityID);
        entity.getComponent<TransformComponent>().setPosition(*translation);
    }

    static bool Input_IsKeyDown(std::uint16_t keycode)
    {
        return robot2D::Keyboard::isKeyPressed(robot2D::Int2Key(keycode));
    }

    template<typename... Component>
    static void RegisterComponent()
    {
        ([]()
        {
            std::string_view typeName = typeid(Component).name();
            size_t pos = typeName.find_last_of(':');
            std::string_view structName = typeName.substr(pos + 1);
            //std::string managedTypename = fmt::format("robot2D.{}", structName);
            std::string managedTypename = "robot2D.TransformComponent";

            MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
            if (!managedType)
            {
              //  HZ_CORE_ERROR("Could not find component type {}", managedTypename);
                return;
            }
            s_EntityHasComponentFuncs[managedType] = [](robot2D::ecs::Entity entity) {
                return entity.hasComponent<Component>();
            };
        }(), ...);
    }


    void ScriptGlue::registerComponents()
    {
        s_EntityHasComponentFuncs.clear();
        RegisterComponent<TransformComponent>();
    }

    void ScriptGlue::registerFunctions()
    {
        RB_ADD_INTERNAL_CALL(NativeLog);
        RB_ADD_INTERNAL_CALL(Entity_HasComponent);
        RB_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        RB_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        RB_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }
} // namespace editor