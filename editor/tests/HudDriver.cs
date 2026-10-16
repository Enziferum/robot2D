using robot2D;
public static class HudDriver
{
    private static HudExample hud;
    public static int Bind(ulong health, ulong wave, ulong status)
    {
        hud = new HudExample { HealthLabel = new Entity(health).GetComponent<TextComponent>(),
            WaveLabel = new Entity(wave).GetComponent<TextComponent>(),
            StatusLabel = new Entity(status).GetComponent<TextComponent>() };
        hud.OnCreate();
        return hud.HealthLabel != null && hud.WaveLabel != null && hud.StatusLabel != null ? 1 : 0;
    }
    public static int Cycle(int phase)
    {
        if(phase == 0) { hud.SetHealth(25); hud.SetWave(3); }
        if(phase == 1) hud.SetHealth(0);
        if(phase == 2) hud.RestartRound();
        return 1;
    }
    public static int Set(string value) { return hud.HealthLabel.TrySetText(value) ? 1 : 0; }
    public static string Read() { return hud.HealthLabel.Text; }
    public static int Detached() { var label = new TextComponent(); return !label.TrySetText("x") && label.Text == "" ? 1 : 0; }
}
