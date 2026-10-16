using System;

namespace robot2D
{
    public abstract class Component
    {
        protected Component()
        { }
        
        internal Component(ulong ID)
        {
            Console.WriteLine($"Component Create with ID - {ID}");
            Entity = new Entity(ID);
        }
        public Entity Entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector2 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector2 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }

        public void addChild(Entity child)
        {
            InternalCalls.TransformComponent_AddChild(Entity.ID, child.ID);
        }
    }
    
    public class RigidBody2D : Component
    {
        public enum BodyType { Static = 0, Dynamic, Kinematic }

        public Vector2 LinearVelocity
        {
            get
            {
                InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
                return velocity;
            }
            set
            {
                InternalCalls.Rigidbody2DComponent_SetLinearVelocity(Entity.ID, ref value);
            }
        }

        public BodyType Type
        {
            get => InternalCalls.Rigidbody2DComponent_GetType(Entity.ID);
            set => InternalCalls.Rigidbody2DComponent_SetType(Entity.ID, value);
        }
		
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
        }

        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
        }

        public void AddForce(Vector2 force)
        {
            InternalCalls.Rigidbody2DComponent_AddForce(Entity.ID, ref force);
        }
    }

    public class DrawableComponent : Component
    {
        public void Flip()
        {
            InternalCalls.DrawableComponent_Flip(Entity.ID);
        }
    }

    public class TextComponent : Component
    {
        // A detached or stale component reads empty; TrySetText reports failure.
        public string Text
        {
            get => Entity == null ? string.Empty : InternalCalls.TextComponent_GetText(Entity.ID);
            set => TrySetText(value);
        }

        public bool TrySetText(string value)
        {
            return Entity != null && InternalCalls.TextComponent_SetText(Entity.ID, value);
        }
    }


    public class Animator : Component
    {
        public bool IsPlaying => Entity != null && InternalCalls.AnimationComponent_IsPlaying(Entity.ID);

        // Repeating the active clip does not restart it. Unknown/invalid clips preserve playback.
        public bool TryPlay(string animation)
        {
            return Entity != null && !string.IsNullOrWhiteSpace(animation) &&
                   InternalCalls.AnimationComponent_TryPlay(Entity.ID, animation);
        }

        public void Stop()
        {
            Stop(null);
        }

        public void Play(string animation)
        {
            TryPlay(animation);
        }

        public void Stop(string animation)
        {
            if(Entity != null)
                InternalCalls.AnimationComponent_Stop(Entity.ID, animation);
        }
    }
    
    public class CameraComponent : Component
    {
        public Vector2 Position
        {
            set
            {
                InternalCalls.CameraComponent_SetPosition(Entity.ID, ref value);
            }
        }

        public Vector2 Size
        {
            get
            {
                InternalCalls.CameraComponent_GetSize(Entity.ID, out Vector2 size);
                return size;
            }
            set => InternalCalls.CameraComponent_SetSize(Entity.ID, ref value);
        }
    }
}