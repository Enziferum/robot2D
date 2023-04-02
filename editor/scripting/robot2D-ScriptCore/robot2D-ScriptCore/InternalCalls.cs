using System;
using System.Runtime.CompilerServices;

namespace robot2D
{
    public static class InternalCalls
    {
        #region EntityBase

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(uint entityID, Type componentType);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_FindEntityByName(string name);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object createEntity();
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool removeEntity(uint entityID);
        
        #endregion
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(uint entityID, out Vector2 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(uint entityID, ref Vector2 translation);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keycode);

        #region Physics2D

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(uint entityID, ref Vector2 impulse, ref Vector2 point, bool wake);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_GetLinearVelocity(uint entityID, out Vector2 linearVelocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        
        internal extern static void Rigidbody2DComponent_SetLinearVelocity(uint entityID, ref Vector2 linearVelocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        
        internal extern static RigidBody2D.BodyType Rigidbody2DComponent_GetType(uint  entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetType(uint entityID, RigidBody2D.BodyType type);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(uint entityID, ref Vector2 impulse, bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_AddForce(uint entityID, ref Vector2 force);
        
        #endregion
    
    }
}