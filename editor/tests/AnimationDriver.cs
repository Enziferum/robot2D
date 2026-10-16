using robot2D;

public static class AnimationDriver
{
    private static Animator Get(ulong id)
    {
        return new Entity(id).GetComponent<Animator>();
    }

    public static int Play(ulong id, string name)
    {
        Animator animator = Get(id);
        return animator != null && animator.TryPlay(name) ? 1 : 0;
    }

    public static int Playing(ulong id)
    {
        Animator animator = Get(id);
        return animator != null && animator.IsPlaying ? 1 : 0;
    }

    public static int Stop(ulong id)
    {
        Animator animator = Get(id);
        animator.Stop();
        return animator.IsPlaying ? 1 : 0;
    }

    public static int LegacyPlay(ulong id, string name)
    {
        Animator animator = Get(id);
        animator.Play(name);
        return animator.IsPlaying ? 1 : 0;
    }

    // Simulate a managed component retained after its native entity has disappeared.
    public static int Probe(ulong id)
    {
        Animator animator = new Animator();
        typeof(Component).GetProperty("Entity").SetValue(animator, new Entity(id), null);
        return animator.TryPlay("Idle") || animator.IsPlaying ? 1 : 0;
    }

    public static int Detached()
    {
        Animator animator = new Animator();
        animator.Play(null);
        animator.Stop();
        return animator.TryPlay("Idle") || animator.IsPlaying ? 1 : 0;
    }
}
