﻿using System;
using System.Runtime.CompilerServices;

namespace robot2D
{
    public static class InternalCalls
    {
        #region EntityBase

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Object_Instantiate(ulong from);
        
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Object_Instantiate_WithPos(ulong from, ref Vector2 position);

        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool removeEntity(ulong entityID);
        
        #endregion

        #region Transform

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector2 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector2 translation);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_AddChild(ulong entityID, ulong childID);
        
        #endregion

        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keycode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMousePressed(MouseButton mouseButton);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 position);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_SetMousePosition(ref Vector2 position);

        #endregion
        
        #region Physics2D

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, 
                                                                                            ref Vector2 point, bool wake);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong entityID, out Vector2 linearVelocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        
        internal extern static void Rigidbody2DComponent_SetLinearVelocity(ulong entityID, ref Vector2 linearVelocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        
        internal extern static RigidBody2D.BodyType Rigidbody2DComponent_GetType(ulong  entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetType(ulong entityID, RigidBody2D.BodyType type);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_AddForce(ulong entityID, ref Vector2 force);
        
        #endregion

        #region Camera

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_SetPosition(ulong entityID, ref Vector2 position);
        
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_GetSize(ulong entityID, out Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_SetSize(ulong entityID, ref Vector2 position);

        
        #endregion
        
        #region Drawable
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DrawableComponent_Flip(ulong entityID);
        
        #endregion

        #region Animation

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationComponent_Play(ulong entityID, string animation);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationComponent_Stop(ulong entityID, string animation);

        #endregion

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SceneManager_LoadScene(string name);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SceneManager_LoadSceneAsync(string name);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Engine_Exit();
    }
}