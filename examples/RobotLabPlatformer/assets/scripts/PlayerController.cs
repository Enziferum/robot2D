using System;
using System.Collections.Generic;
using robot2D;

namespace RobotLabPlatformer
{
    // Stage 1 uses the current frame-based API. Fixed input/OnFixedUpdate is a later engine step.
    public sealed class PlayerController : Entity
    {
        private const float MoveSpeed = 8.0f;       // Physics units (metres per second).
        private const float JumpSpeed = 13.8f;
        private const float MaxFallSpeed = 22.0f;
        private const float PixelsPerMetre = 32.0f;
        private const float ReturnSpeed = 1000.0f; // Transform units (pixels per second).
        private const float ReturnTolerance = 1.5f;

        private RigidBody2D body;
        private TransformComponent transform;
        private DrawableComponent drawable;
        private Vector2 spawn;
        private readonly HashSet<ulong> supports = new HashSet<ulong>();
        private bool jumpWasDown;
        private bool resetWasDown;
        private bool facingLeft;
        private bool returning;

        public override void OnCreate()
        {
            body = GetComponent<RigidBody2D>();
            transform = GetComponent<TransformComponent>();
            drawable = GetComponent<DrawableComponent>();
            if(body == null || transform == null || drawable == null)
                throw new InvalidOperationException("Player requires Transform, RigidBody2D and Drawable components.");
            spawn = transform.Translation;
            supports.Clear();
            jumpWasDown = resetWasDown = facingLeft = returning = false;
        }

        public override void OnUpdate(float dt)
        {
            if(float.IsNaN(dt) || float.IsInfinity(dt) || dt <= 0)
                return;

            bool jump = Input.IsKeyDown(KeyCode.Space) || Input.IsKeyDown(KeyCode.W) || Input.IsKeyDown(KeyCode.Up);
            bool reset = Input.IsKeyDown(KeyCode.R);
            Vector2 position = transform.Translation;
            if(!returning && ((reset && !resetWasDown) || position.Y > 790 || position.X < -180 || position.X > 1390))
            {
                returning = true;
                supports.Clear();
                body.Type = RigidBody2D.BodyType.Kinematic;
                body.LinearVelocity = Vector2.Zero;
            }
            resetWasDown = reset;

            if(returning)
            {
                ReturnToStart(position, dt);
                jumpWasDown = jump; // Holding jump during recovery must not launch on arrival.
                return;
            }

            float axis = 0;
            if(Input.IsKeyDown(KeyCode.A) || Input.IsKeyDown(KeyCode.Left)) axis -= 1;
            if(Input.IsKeyDown(KeyCode.D) || Input.IsKeyDown(KeyCode.Right)) axis += 1;
            if(axis != 0 && (axis < 0) != facingLeft)
            {
                drawable.Flip();
                facingLeft = axis < 0;
            }

            Vector2 velocity = body.LinearVelocity;
            // A side/ceiling contact is never support; a still-reported takeoff contact
            // must not permit a second jump while rising.
            if(jump && !jumpWasDown && supports.Count > 0 && velocity.Y >= -0.1f)
            {
                velocity.Y = -JumpSpeed;
                supports.Clear();
            }
            jumpWasDown = jump;
            body.LinearVelocity = new Vector2(axis * MoveSpeed, Math.Min(velocity.Y, MaxFallSpeed));
        }

        private void ReturnToStart(Vector2 position, float dt)
        {
            Vector2 delta = spawn - position;
            float distance = (float)Math.Sqrt(delta.X * delta.X + delta.Y * delta.Y);
            if(distance <= ReturnTolerance)
            {
                body.LinearVelocity = Vector2.Zero;
                body.Type = RigidBody2D.BodyType.Dynamic;
                returning = false;
                return;
            }
            // Dynamic Transform setters do not teleport Box2D. Move the kinematic body
            // through the supported velocity API, then restore normal gravity/collisions.
            float seconds = Math.Max(dt, distance / ReturnSpeed);
            body.LinearVelocity = new Vector2(delta.X / seconds / PixelsPerMetre, delta.Y / seconds / PixelsPerMetre);
        }

        private void UpdateSupport(ulong other, Vector2 normalToMe)
        {
            if(!returning && normalToMe.Y < -0.5f)
                supports.Add(other);
            else
                supports.Remove(other);
        }

        public override void OnCollisionEnter2D(ulong other, ref PhysicsContact2D contact, Vector2 normalToMe)
        {
            UpdateSupport(other, normalToMe);
        }

        public override void OnCollisionStay2D(ulong other, ref PhysicsContact2D contact, Vector2 normalToMe)
        {
            UpdateSupport(other, normalToMe);
        }

        public override void OnCollisionExit2D(ulong other, ref PhysicsContact2D contact)
        {
            supports.Remove(other);
        }
    }
}
