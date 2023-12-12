using System;
using System.Collections;

namespace robot2D
{
    public class Collider2D: Component
    {
        
    }
    
    public class Collision2D
    {
        private Entity m_collider;
        private Entity m_otherCollider;

        internal Collision2D(ulong collider, ulong otherCollider)
        {
            m_collider = new Entity(collider);
            m_otherCollider = new Entity(otherCollider);
        }
        
        public Collider2D collider
        {
            get
            {
                try
                {
                    return m_collider.GetComponent<Collider2D>();
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
                    return m_otherCollider.GetComponent<Collider2D>();
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
                    return m_collider.GetComponent<RigidBody2D>();
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
                    return m_otherCollider.GetComponent<RigidBody2D>();
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