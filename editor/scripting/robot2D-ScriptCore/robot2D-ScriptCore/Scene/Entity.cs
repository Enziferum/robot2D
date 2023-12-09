using System;
using System.Dynamic;
using System.Reflection;
using System.Reflection.Emit;

namespace robot2D
{
    
    public static class Object
    {
        public static object FindObjectFromInstanceID(ulong ID)
        {
            return InternalCalls.GetScriptInstance(ID);
        }
    }

    internal class ComponentCreator
    {
        public T Create<T>(ulong UUID) where T: Component, new()
        {
            T component = new T() { Entity = new Entity(UUID) };
            return component;
        }
    }
    
    public class Entity: System.Object
    {
        protected Entity()
        {
            Console.WriteLine("Entity Default Ctor");
            ID = 0;
        } 

        internal Entity(ulong id)
        {
            Console.WriteLine($"Entity Ctor ID - {id}");
            ID = id;
        }

        internal void setComponentField(string name, ulong uuid)
        {
            Type t = GetType();
            FieldInfo fieldInfo = t.GetRuntimeField(name);
            
            if (fieldInfo != null)
            {
                ComponentCreator componentCreator = new ComponentCreator();
                MethodInfo method = typeof(ComponentCreator).GetMethod(nameof(ComponentCreator.Create));
                MethodInfo generic = method.MakeGenericMethod(fieldInfo.FieldType);
                var component = generic.Invoke(componentCreator, new object[]{ uuid });
                fieldInfo.SetValue(this, component);
            }
                
        }
        
        public readonly ulong ID;

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
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
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