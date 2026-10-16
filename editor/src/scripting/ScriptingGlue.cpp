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

#include <unordered_map>
#include <robot2D/Core/Keyboard.hpp>
#include <robot2D/Core/Mouse.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Core/Assert.hpp>
#include <robot2D/Core/Window.hpp>

#include <mono/metadata/loader.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <editor/scripting/ScriptingGlue.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/Scene.hpp>
#include <editor/Components.hpp>
#include <editor/AnimationPlayback.hpp>
#include <editor/Uuid.hpp>

namespace editor {

    namespace Utils {

        inline b2BodyType Rigidbody2DTypeToBox2DBody(Physics2DComponent::BodyType bodyType) {
            switch(bodyType)
            {
            case Physics2DComponent::BodyType::Static:    return b2_staticBody;
            case Physics2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
            case Physics2DComponent::BodyType::Kinematic:   return b2_kinematicBody;
            }

            return b2_staticBody;
        }

        inline Physics2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType) {
            switch(bodyType)
            {
            case b2_staticBody:    return Physics2DComponent::BodyType::Static;
            case b2_dynamicBody:   return Physics2DComponent::BodyType::Dynamic;
            case b2_kinematicBody:   return Physics2DComponent::BodyType::Kinematic;
            }

            return Physics2DComponent::BodyType::Static;
        }

        std::string MonoStringToString(MonoString* string) {
            char* cStr = mono_string_to_utf8(string);
            std::string str(cStr);
            mono_free(cStr);
            return str;
        }

    }


    #define RB_ADD_INTERNAL_CALL(Name) mono_add_internal_call("robot2D.InternalCalls::" #Name, (void*)Name)

    static std::unordered_map<MonoType*, std::function<bool(SceneEntity)> > s_EntityHasComponentFuncs;

    template<typename T>
    std::string combineTypeName(std::string namespaceName = "robot2D") {
        std::string resultType = namespaceName + '.';
        resultType += T::id().getName();
        return resultType;
    }


    static void NativeLog(MonoString* string, int parameter) {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        std::cout << str << ", " << parameter << std::endl;
    }

    static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        MonoType* managedType = mono_reflection_type_get_type(componentType);
        std::string typeName = mono_type_get_name(managedType);

        RB_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
        bool hasComp = s_EntityHasComponentFuncs.at(managedType)(entity);
        return hasComp;
    }

    static std::uint64_t Entity_FindEntityByName(MonoString* name) {
        auto service = ScriptGlue::getService();
        auto cppString = Utils::MonoStringToString(name);
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(cppString);
        RB_CORE_ASSERT(entity);
        auto uuid = entity.getUUID();
        auto myUuid = static_cast<std::uint64_t>(uuid);
        return myUuid;
    }

    static void TransformComponent_GetTranslation(UUID entityID, robot2D::vec2f* outTranslation) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        *outTranslation = entity.getComponent<TransformComponent>().getPosition();
    }

    static void TransformComponent_SetTranslation(UUID entityID, robot2D::vec2f* translation) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        entity.getComponent<TransformComponent>().setPosition(*translation);
        if(entity.hasComponent<Physics2DComponent>()) {
            auto& p2c = entity.getComponent<Physics2DComponent>();
            if(p2c.type == Physics2DComponent::BodyType::Kinematic) {

                b2Body* body = static_cast<b2Body*>(p2c.runtimeBody);
                body -> SetTransform({ (*translation).x / 32, (*translation).y / 32 }, 0.f);
            }
        }

    }

    static void TransformComponent_AddChild(UUID entityID, UUID child) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        SceneEntity childEntity = interactor -> getEntity(child);
        RB_CORE_ASSERT(childEntity);

        entity.addChild(childEntity);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, robot2D::vec2f* impulse, robot2D::vec2f* point,
        bool wake) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> ApplyLinearImpulse( b2Vec2(impulse -> x, impulse -> y), b2Vec2(point->x, point->y), wake);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, robot2D::vec2f* impulse, bool wake) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
    }

    static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, robot2D::vec2f* outLinearVelocity) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        const b2Vec2& linearVelocity = body->GetLinearVelocity();
        *outLinearVelocity = robot2D::vec2f(linearVelocity.x, linearVelocity.y);

    }

    static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, robot2D::vec2f* velocity) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = static_cast<b2Body*>(rb2d.runtimeBody);
        body -> SetLinearVelocity({velocity -> x, velocity -> y});
    }

    static Physics2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);


        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        return Utils::Rigidbody2DTypeFromBox2DBody( body -> GetType());
    }

    static void Rigidbody2DComponent_SetType(UUID entityID, Physics2DComponent::BodyType bodyType) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        body -> SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
    }

    static void RigidBody2DComponent_AddForce(UUID entityID, robot2D::vec2f* force) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& rb2d = entity.getComponent<Physics2DComponent>();
        b2Body* body = (b2Body*)rb2d.runtimeBody;
        body -> ApplyForceToCenter({force -> x, force -> y}, true);
    }

    static void CameraComponent_SetPosition(UUID entityID, robot2D::vec2f* position) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& cameraComponent = entity.getComponent<CameraComponent>();
        cameraComponent.position = *position;
        entity.getComponent<TransformComponent>().setPosition(*position);
    }

    static void CameraComponent_GetSize(UUID entityID, robot2D::vec2f* size) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& cameraComponent = entity.getComponent<CameraComponent>();
        *size = cameraComponent.size;
    }

    static void CameraComponent_SetSize(UUID entityID, robot2D::vec2f* size) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& cameraComponent = entity.getComponent<CameraComponent>();
        cameraComponent.size = *size;
    }

    static void DrawableComponent_Flip(UUID entityID) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        SceneEntity entity = interactor -> getEntity(entityID);
        RB_CORE_ASSERT(entity);

        auto& drawable = entity.getComponent<DrawableComponent>();
        drawable.FlipTexture();
    }


    static SceneEntity scriptEntity(UUID entityID) {
        auto* service = ScriptGlue::getService();
        if(!service)
            return {};
        auto interactor = service -> getInteractor();
        return interactor ? interactor -> getEntity(entityID) : SceneEntity{};
    }

    static MonoString* TextComponent_GetText(UUID entityID) {
        auto entity = scriptEntity(entityID);
        if(!entity || !entity.hasComponent<TextComponent>())
            return mono_string_new(mono_domain_get(), "");
        const auto& text = entity.getComponent<TextComponent>().getText();
        return mono_string_new_len(mono_domain_get(), text.data(), static_cast<unsigned int>(text.size()));
    }

    static bool TextComponent_SetText(UUID entityID, MonoString* value) {
        auto entity = scriptEntity(entityID);
        if(!entity || !entity.hasComponent<TextComponent>())
            return false;
        if(value) {
            const auto* characters = mono_string_chars(value);
            for(int i = 0; i < mono_string_length(value); ++i)
                if(characters[i] == 0) return false;
        }
        entity.getComponent<TextComponent>().setText(value ? Utils::MonoStringToString(value) : std::string{});
        return true;
    }

    static bool AnimationComponent_TryPlay(UUID entityID, MonoString* name) {
        if(!name)
            return false;
        const auto clipName = Utils::MonoStringToString(name);
        const bool result = playAnimation(scriptEntity(entityID), clipName);
        if(!result)
            RB_EDITOR_WARN(
                "Animator::TryPlay: entity={0}, clip={1}, reason=entity, component or clip unavailable/invalid",
                static_cast<std::uint64_t>(entityID), clipName);
        return result;
    }

    static void AnimationComponent_Play(UUID entityID, MonoString* name) {
        AnimationComponent_TryPlay(entityID, name);
    }

    static void AnimationComponent_Stop(UUID entityID, MonoString* name) {
        stopAnimation(scriptEntity(entityID), name ? Utils::MonoStringToString(name) : std::string{});
    }

    static bool AnimationComponent_IsPlaying(UUID entityID) {
        auto entity = scriptEntity(entityID);
        return entity && entity.hasComponent<AnimatorComponent>() &&
               entity.getComponent<AnimatorComponent>().playing();
    }

    static bool Input_IsKeyDown(std::uint16_t keycode) {
        return robot2D::Keyboard::isKeyPressed(robot2D::Int2Key(keycode));
    }

    static bool Input_IsMousePressed(std::uint16_t button) {
        return robot2D::isMousePressed(robot2D::int2mouse(button));
    }

    static void Input_GetMousePosition(robot2D::vec2f* position) {
        auto service = ScriptGlue::getService();
        auto* window = service -> GetWindow();
        auto camera = service -> GetCamera();

        auto mousePos = window -> getMousePos();
        mousePos = camera -> convertPixelToCoords(mousePos);
        *position = mousePos;
    }

    static void Input_SetMousePosition(robot2D::vec2f* position) {
        auto service = ScriptGlue::getService();
        auto* window = service -> GetWindow();
        window -> setMousePos(*position);
    }

    static bool SceneManager_LoadScene(MonoString* name) {
        auto cppString = Utils::MonoStringToString(name);
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);

        return interactor -> loadSceneRuntime(std::move(cppString));
    }

    static void SceneManager_LoadSceneAsync(MonoString* name) {
        auto cppString = Utils::MonoStringToString(name);
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        interactor -> loadSceneAsyncRuntime(std::move(cppString));
    }

    static void Engine_Exit() {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        interactor -> exitEngineRuntime();
    }

    static void Object_Instantiate(UUID uuid) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        auto entity = interactor -> getEntity(uuid);
        RB_CORE_ASSERT(entity);

        interactor -> duplicateRuntime(entity);
    }

    static void Object_Instantiate_WithPos(UUID uuid, robot2D::vec2f* position) {
        auto service = ScriptGlue::getService();
        auto interactor = service -> getInteractor();
        RB_CORE_ASSERT(interactor);
        auto entity = interactor -> getEntity(uuid);
        RB_CORE_ASSERT(entity);

        robot2D::vec2f dupPosition = *position;
        auto dupEntity = interactor -> duplicateRuntime(entity, dupPosition);
        service -> onCreateEntity(dupEntity);
    }

    static MonoObject* GetScriptInstance(UUID uuid) {
        auto service = ScriptGlue::getService();
        return service -> getManagedObject(uuid) -> getInstance();
    }

    template<typename Component>
    static void RegisterComponent(ScriptingEngineInternalService* service) {
        std::string managedTypename = combineTypeName<Component>();
        MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(),
                service -> GetCoreAssemblyImage());
        if(!managedType) {
            RB_CORE_ERROR("ScriptingGlue: Could not find component type {0}", managedTypename);
            return;
        }
        s_EntityHasComponentFuncs[managedType] = [](SceneEntity entity) {
            return entity.hasComponent<Component>();
        };
    }

    void ScriptGlue::setService(ScriptingEngineInternalService* service) {
        m_service = service;
    }

    ScriptingEngineInternalService* ScriptGlue::getService() {
        return m_service;
    }


    void ScriptGlue::registerComponents() {
        s_EntityHasComponentFuncs.clear();
        RegisterComponent<TransformComponent>(m_service);
        RegisterComponent<Physics2DComponent>(m_service);
        RegisterComponent<Collider2DComponent>(m_service);
        RegisterComponent<CameraComponent>(m_service);
        RegisterComponent<TextComponent>(m_service);
        RegisterComponent<DrawableComponent>(m_service);
        RegisterComponent<AnimatorComponent>(m_service);
        RegisterComponent<PhysicsContact2D>(m_service);
        RegisterComponent<PhysicsContactPoint2D>(m_service);
    }

    void ScriptGlue::registerFunctions() {
        RB_ADD_INTERNAL_CALL(NativeLog);
        RB_ADD_INTERNAL_CALL(GetScriptInstance);
        RB_ADD_INTERNAL_CALL(Entity_HasComponent);
        RB_ADD_INTERNAL_CALL(Entity_FindEntityByName);
        RB_ADD_INTERNAL_CALL(Input_IsKeyDown);
        RB_ADD_INTERNAL_CALL(Input_IsMousePressed);
        RB_ADD_INTERNAL_CALL(Input_GetMousePosition);
        RB_ADD_INTERNAL_CALL(Input_SetMousePosition);
        RB_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        RB_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        RB_ADD_INTERNAL_CALL(TransformComponent_AddChild);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetLinearVelocity);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
        RB_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
        RB_ADD_INTERNAL_CALL(RigidBody2DComponent_AddForce);
        RB_ADD_INTERNAL_CALL(CameraComponent_SetPosition);
        RB_ADD_INTERNAL_CALL(CameraComponent_GetSize);
        RB_ADD_INTERNAL_CALL(CameraComponent_SetSize);
        RB_ADD_INTERNAL_CALL(DrawableComponent_Flip);
        RB_ADD_INTERNAL_CALL(TextComponent_GetText);
        RB_ADD_INTERNAL_CALL(TextComponent_SetText);
        RB_ADD_INTERNAL_CALL(AnimationComponent_Play);
        RB_ADD_INTERNAL_CALL(AnimationComponent_TryPlay);
        RB_ADD_INTERNAL_CALL(AnimationComponent_IsPlaying);
        RB_ADD_INTERNAL_CALL(AnimationComponent_Stop);
        RB_ADD_INTERNAL_CALL(SceneManager_LoadScene);
        RB_ADD_INTERNAL_CALL(SceneManager_LoadSceneAsync);
        RB_ADD_INTERNAL_CALL(Engine_Exit);
        RB_ADD_INTERNAL_CALL(Object_Instantiate);
        RB_ADD_INTERNAL_CALL(Object_Instantiate_WithPos);
    }


} // namespace editor