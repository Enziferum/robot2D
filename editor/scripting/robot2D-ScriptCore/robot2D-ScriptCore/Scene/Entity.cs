﻿using System;
using System.Runtime.CompilerServices;

namespace robot2D
{
    public class Entity
    {
        protected Entity()
        {
            Console.WriteLine("Entity Default Ctor");
            ID = 0;
        } 

        internal Entity(int id)
        {
            Console.WriteLine($"Entity Ctor ID - {id}");
            ID = id;
            Console.WriteLine($"Entity Ctor ID - {ID}");
        }

        public readonly int ID;

        public Vector2 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector2 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(ID, ref value);
            }
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }
		
        public Entity FindEntityByName(string name)
        {
            int entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.GetScriptInstance(ID);
            return instance as T;
        }

    }

}