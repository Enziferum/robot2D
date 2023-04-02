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

#include <box2d/box2d.h>

namespace editor {

    namespace Utils {

        inline b2BodyType Rigidbody2DTypeToBox2DBody(Physics2DComponent::BodyType bodyType)
        {
            switch (bodyType)
            {
                case Physics2DComponent::BodyType::Static:    return b2_staticBody;
                case Physics2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
                case Physics2DComponent::BodyType::Kinematic:   return b2_kinematicBody;
            }

            return b2_staticBody;
        }

        inline Physics2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
        {
            switch (bodyType)
            {
                case b2_staticBody:    return Physics2DComponent::BodyType::Static;
                case b2_dynamicBody:   return Physics2DComponent::BodyType::Dynamic;
                case b2_kinematicBody:   return Physics2DComponent::BodyType::Kinematic;
            }

            return Physics2DComponent::BodyType::Static;
        }
    }

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

    static void Rigidbody2DComponent_ApplyLinearImpulse(std::uint32_t entityID, robot2D::vec2f* impulse, robot2D::vec2f* point, bool wake)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> ApplyLinearImpulse( b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(std::uint32_t entityID, robot2D::vec2f* impulse, bool wake)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
    }

    static void Rigidbody2DComponent_GetLinearVelocity(std::uint32_t entityID,  robot2D::vec2f* outLinearVelocity)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);
        //HZ_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        const b2Vec2& linearVelocity = body->GetLinearVelocity();
        *outLinearVelocity = robot2D::vec2f(linearVelocity.x, linearVelocity.y);

    }

    static void Rigidbody2DComponent_SetLinearVelocity(std::uint32_t entityID, robot2D::vec2f* velocity)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);
        //HZ_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> SetLinearVelocity({velocity -> x, velocity -> y});
    }

    static Physics2DComponent::BodyType Rigidbody2DComponent_GetType(std::uint32_t entityID)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        return Utils::Rigidbody2DTypeFromBox2DBody( body -> GetType());
    }

    static void Rigidbody2DComponent_SetType(std::uint32_t entityID, Physics2DComponent::BodyType bodyType)
    {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        body -> SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
    }

    static void RigidBody2DComponent_AddForce(std::uint32_t entityID, robot2D::vec2f* force) {
        Scene* scene = ScriptEngine::getSceneContext();
        auto entity = scene -> getByIndex(entityID);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        body -> ApplyForceToCenter({force -> x, force -> y}, true);
    }

    static bool Input_IsKeyDown(std::uint16_t keycode)
    {
        return robot2D::Keyboard::isKeyPressed(robot2D::Int2Key(keycode));
    }

    static MonoObject* GetScriptInstance(std::uint32_t entityID) {
        return ScriptEngine::getManagedObject(entityID) -> getInstance();
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
            std::string managedTypename = "robot2D.RigidBody2D";

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
      //  RegisterComponent<TransformComponent>();
        RegisterComponent<Physics2DComponent>();
    }

    void ScriptGlue::registerFunctions()
    {
        RB_ADD_INTERNAL_CALL(NativeLog);
        RB_ADD_INTERNAL_CALL(GetScriptInstance);
        RB_ADD_INTERNAL_CALL(Entity_HasComponent);
        RB_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        RB_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        RB_ADD_INTERNAL_CALL(Input_IsKeyDown);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetLinearVelocity);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
    }
} // namespace editor