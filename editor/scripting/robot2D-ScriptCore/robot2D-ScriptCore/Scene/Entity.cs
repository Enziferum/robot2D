using System;
using System.Collections.Specialized;
using System.Dynamic;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;

namespace robot2D
{

    public enum PhysicsEventType : byte
    {
        Enter, Stay, Exit, EnterTrigger, StayTrigger, ExitTrigger
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PhysicsContactPoint2D { public float px, py, sep; }

    [StructLayout(LayoutKind.Sequential)]
    public struct PhysicsContact2D
    {
        public ulong entityA, entityB;
        public ulong fixtureA, fixtureB;
        public PhysicsEventType type;
        public float nx, ny;
        public PhysicsContactPoint2D p0, p1;
        public byte pointCount;
        public float normalImpulse, tangentImpulse;
        [MarshalAs(UnmanagedType.I1)] public bool isSensorA, isSensorB;
        public ushort categoryA, categoryB, maskA, maskB;
        //public float relVelX, relVelY;
        //public uint frameIndex;
    }

    public static class Object
    {
        public static object FindObjectFromInstanceID(ulong ID)
        {
            return InternalCalls.GetScriptInstance(ID);
        }

        public static void Instantiate(Entity from)
        {
            InternalCalls.Object_Instantiate(from.ID);
        }
        
        public static void Instantiate(Entity from, Vector2 position)
        {
            InternalCalls.Object_Instantiate_WithPos(from.ID, ref position);
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

    internal class EntityCreator
    {
        public T Create<T>() where T: Entity, new()
        {
            T entity = (T)Activator.CreateInstance(typeof(T));
            return entity;
        }
    }
    
    public class Entity: System.Object
    {
        protected Entity()
        {
            Console.WriteLine("Entity Default Ctor");
            ID = 0;
        } 

        public Entity(ulong id)
        {
            Console.WriteLine($"Entity Ctor ID - {id}");
            ID = id;
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
        
        internal void setEntityField(string name, ulong uuid)
        {
            Type t = GetType();
            FieldInfo fieldInfo = t.GetRuntimeField(name);
            
            if (fieldInfo != null)
            {
                Type baseType = fieldInfo.FieldType.BaseType;
                Console.WriteLine($"setEntityField::calls and basetype is {baseType.ToString()}");
                foreach (var MyConstructor in baseType.GetConstructors())
                {
                    Console.WriteLine($"for type {baseType.ToString()}, " +
                                      $"constructor has {MyConstructor.GetParameters().Length} params");
                    if (MyConstructor.GetParameters().Length != 0)
                    {
                        EntityCreator entityCreator = new EntityCreator();
                        MethodInfo method = typeof(EntityCreator).GetMethod(nameof(EntityCreator.Create));
                        MethodInfo generic = method.MakeGenericMethod(fieldInfo.FieldType);
                        var entity = generic.Invoke(entityCreator, new object[]{ });
                        Console.WriteLine($"setEntityField::Base Constructor Call to uuid {uuid}");
                        MyConstructor.Invoke(entity, new object[] { uuid });
                        fieldInfo.SetValue(this, entity);
                        break;
                    }
                }
            }
                
        }

        /*
        internal void onCollision2DInternal(ulong entityID, ulong otherEntityID, int type)
        {
            Collision2D collision2D = new Collision2D(entityID, otherEntityID);
            string methodName = "None";
            switch (type)
            {
                case 0:
                    methodName = "onCollision2DEnter";
                    break;
                case 1:
                    methodName = "onCollision2DExit";
                    break;
                case 2:
                    methodName = "onCollision2DEnterTrigger";
                    break;
                case 3:
                    methodName = "onCollision2DExitTrigger";
                    break;
            }
            var method = GetType().GetMethod(methodName);
            if(method != null)
                method.Invoke(this, new object[] { collision2D });
        }
        */

        public virtual void OnCreate() {}
        public virtual void OnUpdate(float deltaTime) { }
        public virtual void OnCollision2DInternal(ulong self, ulong other, int typeInt, ref PhysicsContact2D ev)
        {
            Console.WriteLine("Entity: OnCollision2DInternal");
            var type = (PhysicsEventType)typeInt;

            // нормаль "к нам"
            var nToMe = (self == ev.entityA)
                ? new Vector2(-ev.nx, -ev.ny)
                : new Vector2(ev.nx, ev.ny);

            switch (type)
            {
                case PhysicsEventType.Enter: OnCollisionEnter2D(other, ref ev, nToMe); break;
                case PhysicsEventType.Stay: OnCollisionStay2D(other, ref ev, nToMe); break;
                case PhysicsEventType.Exit: OnCollisionExit2D(other, ref ev); break;
                case PhysicsEventType.EnterTrigger: OnTriggerEnter2D(other, ref ev); break;
                case PhysicsEventType.StayTrigger: OnTriggerStay2D(other, ref ev); break;
                case PhysicsEventType.ExitTrigger: OnTriggerExit2D(other, ref ev); break;
            }
        }
        

        public virtual void OnCollisionEnter2D(ulong other, ref PhysicsContact2D ev, Vector2 normalToMe) { }
        public virtual void OnCollisionStay2D(ulong other, ref PhysicsContact2D ev, Vector2 normalToMe) { }
        public virtual void OnCollisionExit2D(ulong other, ref PhysicsContact2D ev) { }
        public virtual void OnTriggerEnter2D(ulong other, ref PhysicsContact2D ev) { }
        public virtual void OnTriggerStay2D(ulong other, ref PhysicsContact2D ev) { }
        public virtual void OnTriggerExit2D(ulong other, ref PhysicsContact2D ev) { }
    }
}