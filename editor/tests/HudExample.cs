using robot2D;

// Assign three TextComponent fields in Inspector. The example owns only its demo state.
public sealed class HudExample : Entity
{
    public TextComponent HealthLabel;
    public TextComponent WaveLabel;
    public TextComponent StatusLabel;
    private int health = 100;
    private int wave = 1;
    private bool restartHeld;

    public override void OnCreate() { RestartRound(); }
    public void SetHealth(int value) { health = System.Math.Max(0, value); Refresh(); }
    public void SetWave(int value) { wave = System.Math.Max(1, value); Refresh(); }
    public void RestartRound() { health = 100; wave = 1; Refresh(); }
    public override void OnUpdate(float dt)
    {
        bool down = Input.IsKeyDown(KeyCode.R);
        if(down && !restartHeld) RestartRound();
        restartHeld = down;
    }
    private void Refresh()
    {
        if(HealthLabel != null) HealthLabel.Text = "Health: " + health;
        if(WaveLabel != null) WaveLabel.Text = "Wave: " + wave;
        if(StatusLabel != null) StatusLabel.Text = health == 0 ? "Game Over - press R" : string.Empty;
    }
}
