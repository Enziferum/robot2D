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

#include <functional>
#include <unordered_map>
#include <robot2D/Core/Keyboard.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Core/Assert.hpp>

#include <mono/metadata/loader.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <editor/scripting/ScriptingGlue.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/Scene.hpp>
#include <editor/Components.hpp>
#include <editor/Uuid.hpp>

/// TODO(a.raag): include PhysicsAdapter
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

    /// TODO(a.raag): refactor to be more useful
    template<typename T>
    std::string combineTypeName(std::string namespaceName = "robot2D") {
//        std::string dirtyTypeName = typeid(T).name();
//        auto pos = dirtyTypeName.find(namespaceName);
//        if(pos == std::string::npos)
//            return "";
//        auto resultType = dirtyTypeName.substr(pos, namespaceName.length());
//        resultType += '.';
//        pos = dirtyTypeName.find(typeName);
//        if(pos == std::string::npos)
//            return "";
//        resultType += dirtyTypeName.substr(pos, typeName.length());

        std::string resultType = namespaceName + '.';
        resultType += T::id().getName();

        return resultType;
    }


    static void NativeLog(MonoString* string, int parameter)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        std::cout << str << ", " << parameter << std::endl;
    }

    static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);

        MonoType* managedType = mono_reflection_type_get_type(componentType);
        std::string typeName = mono_type_get_name(managedType);
        RB_EDITOR_INFO("ScriptingGlue: Has Component From C# ? {0}", typeName);
        RB_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
        bool hasComp = s_EntityHasComponentFuncs.at(managedType)(entity);
        return hasComp;
    }

    static void TransformComponent_GetTranslation(UUID entityID, robot2D::vec2f* outTranslation)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);

        *outTranslation = entity.getComponent<TransformComponent>().getPosition();
    }

    static void TransformComponent_SetTranslation(UUID entityID, robot2D::vec2f* translation)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);

        entity.getComponent<TransformComponent>().setPosition(*translation);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, robot2D::vec2f* impulse, robot2D::vec2f* point, bool wake)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> ApplyLinearImpulse( b2Vec2(impulse -> x, impulse -> y), b2Vec2(point->x, point->y), wake);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, robot2D::vec2f* impulse, bool wake)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
    }

    static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID,  robot2D::vec2f* outLinearVelocity)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        const b2Vec2& linearVelocity = body->GetLinearVelocity();
        *outLinearVelocity = robot2D::vec2f(linearVelocity.x, linearVelocity.y);

    }

    static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, robot2D::vec2f* velocity)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> SetLinearVelocity({velocity -> x, velocity -> y});
    }

    static Physics2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        return Utils::Rigidbody2DTypeFromBox2DBody( body -> GetType());
    }

    static void Rigidbody2DComponent_SetType(UUID entityID, Physics2DComponent::BodyType bodyType)
    {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);



        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        body -> SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
    }

    static void RigidBody2DComponent_AddForce(UUID entityID, robot2D::vec2f* force) {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
         RB_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        body -> ApplyForceToCenter({force -> x, force -> y}, true);
    }

    static void CameraComponent_SetPosition(UUID entityID, robot2D::vec2f* position) {
        auto interactor = ScriptEngine::getInteractor();
        RB_CORE_ASSERT(interactor);
        robot2D::ecs::Entity entity = interactor -> getByUUID(entityID);
        RB_CORE_ASSERT(entity);

        auto& cameraComponent = entity.getComponent<CameraComponent>();
        cameraComponent.position = *position;
    }

    static bool Input_IsKeyDown(std::uint16_t keycode)
    {
        return robot2D::Keyboard::isKeyPressed(robot2D::Int2Key(keycode));
    }

    static MonoObject* GetScriptInstance(UUID entityID) {
        return ScriptEngine::getManagedObject(entityID) -> getInstance();
    }

        #define STR_COMPONENT(component) #component

    template<typename... Component>
    static void RegisterComponents()
    {
        ([]()
        {

            std::string managedTypename = combineTypeName<Component>(STR_COMPONENT(Component));
            if(managedTypename == "robot2D.Physics2DComponent")
                managedTypename = "robot2D.RigidBody2D";

            MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
            if (!managedType)
            {
                RB_CORE_ERROR("ScriptingGlue: Could not find component type {0}", managedTypename);
                return;
            }
            s_EntityHasComponentFuncs[managedType] = [](robot2D::ecs::Entity entity) {
                return entity.hasComponent<Component>();
            };
        }(), ...);
    }

    template<typename Component>
    static void RegisterComponent() {
        std::string managedTypename = combineTypeName<Component>();
        MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(),
                                                               ScriptEngine::GetCoreAssemblyImage());
        if (!managedType)
        {
            RB_CORE_ERROR("ScriptingGlue: Could not find component type {0}", managedTypename);
            return;
        }
        s_EntityHasComponentFuncs[managedType] = [](robot2D::ecs::Entity entity) {
            return entity.hasComponent<Component>();
        };
    }


    void ScriptGlue::registerComponents()
    {
        s_EntityHasComponentFuncs.clear();
        RegisterComponent<TransformComponent>();
        RegisterComponent<Physics2DComponent>();
        RegisterComponent<CameraComponent>();
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
        RB_ADD_INTERNAL_CALL(CameraComponent_SetPosition);
    }
} // namespace editor