using System;
using System.IO;
using robot2D;
public sealed class PlayerSmoke : Entity
{
    public int StartingWave;
    private RigidBody2D body;
    private TextComponent label;
    private Animator animator;
    private float startX;
    private int ticks;
    private int enters;
    public override void OnCreate()
    {
        if(StartingWave != 3) throw new Exception("Serialized script field lost");
        body = GetComponent<RigidBody2D>();
        body.LinearVelocity = new Vector2(1, 0);
        animator = GetComponent<Animator>();
        if(!animator.TryPlay("Idle")) throw new Exception("Animation clip missing");
        label = FindEntityByName("HUD").GetComponent<TextComponent>();
        label.Text = "Health: 100 / Wave: " + StartingWave;
        startX = Translation.X;
        Vector2 mouse = Input.GetMousePosition();
        if(float.IsNaN(mouse.X) || float.IsNaN(mouse.Y)) throw new Exception("Invalid player mouse coordinates");
        Input.IsKeyDown(KeyCode.R);
        Input.IsMousePressed(MouseButton.Left);
    }
    public override void OnUpdate(float dt)
    {
        if(++ticks < 12) return;
        if(Translation.X <= startX || enters != 1 || !animator.IsPlaying || label.Text != "Health: 100 / Wave: 3")
            throw new Exception("Physics/trigger/animation/HUD did not update");
        string report = Environment.GetEnvironmentVariable("RB2D_PLAYER_TEST_REPORT");
        if(string.IsNullOrEmpty(report)) return; // Interactive fixture stays open until the window is closed.
        File.WriteAllText(report, "PASS " + StartingWave + " " + ticks);
        Engine.ExitEngine();
    }
    public override void OnTriggerEnter2D(ulong other, ref PhysicsContact2D ev) { ++enters; }
}
