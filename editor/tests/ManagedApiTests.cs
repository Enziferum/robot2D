using System;
using robot2D;

// Compiling this class exercises the public API used by an animation script.
public sealed class AnimationApiConsumer : Entity
{
    public void SelectClip(string name)
    {
        Animator animator = GetComponent<Animator>();
        animator.Play(name);
        animator.Stop(name);
    }
}

internal static class ManagedApiTests
{
    private static int Main()
    {
        // Load the actual freshly built API through Mono; native calls belong to
        // separate runtime tests and are deliberately not invoked without a scene.
        var assembly = typeof(Entity).Assembly;
        if(assembly.GetName().Name != "robot2D_ScriptCore")
            return 1;
        if(!typeof(Component).IsAssignableFrom(typeof(Animator)))
            return 2;
        if(typeof(Animator).GetMethod("Play", new[] { typeof(string) }) == null ||
           typeof(Animator).GetMethod("Stop", new[] { typeof(string) }) == null)
            return 3;
        Console.WriteLine("ScriptCore loaded and animation API consumer compiled successfully.");
        return 0;
    }
}
