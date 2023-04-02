using System;
using System.Collections;

namespace robot2D
{
    public class Collider2D: Component
    {
        
    }
    
    public class Collision2D
    {
        internal uint m_collider;
        
        internal uint m_otherCollider;

        public Collider2D collider
        {
            get
            {
                try
                {
                    var ent = Object.FindObjectFromInstanceID(m_collider) as Entity;
                    return ent.GetComponent<Collider2D>();
                }
                catch (NullReferenceException e)
                {
                    Console.WriteLine(e);
                    throw;
                }
            }
        }
        
        public Collider2D otherCollider
        {
            get
            {
                try
                {
                    var ent = Object.FindObjectFromInstanceID(m_collider) as Entity;
                    return ent.GetComponent<Collider2D>();
                }
                catch (NullReferenceException e)
                {
                    Console.WriteLine(e);
                    throw;
                }
            }
        }
        
        public RigidBody2D rigidBody
        {
            get
            {
                try
                {
                    var ent = Object.FindObjectFromInstanceID(m_collider) as Entity;
                    return ent.GetComponent<RigidBody2D>();
                }
                catch (NullReferenceException e)
                {
                    Console.WriteLine(e);
                    throw;
                }
            }
        }
        
        public RigidBody2D otherRigidBody
        {
            get
            {
                try
                {
                    var ent = Object.FindObjectFromInstanceID(m_collider) as Entity;
                    return ent.GetComponent<RigidBody2D>();
                }
                catch (NullReferenceException e)
                {
                    Console.WriteLine(e);
                    throw;
                }
            }
        }
    }
}