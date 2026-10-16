using System;
using System.Runtime.InteropServices;
using robot2D;

public sealed class PhysicsReceiver : Entity
{
    public int EnterCount, StayCount, ExitCount, TriggerEnterCount, Errors;
    public float NormalX, NormalY;
    public ulong LastOther;
    public override void OnCollisionEnter2D(ulong other, ref PhysicsContact2D ev, Vector2 normalToMe)
    {
        ++EnterCount;
        LastOther = other;
        NormalX = normalToMe.X; NormalY = normalToMe.Y;
        if(other == ID || (ev.entityA != ID && ev.entityB != ID) || ev.fixtureA == 0 || ev.fixtureB == 0 ||
           ev.pointCount > 2 || Math.Abs(ev.nx) > 1.01f || Math.Abs(ev.ny) > 1.01f)
            ++Errors;
        // Native must isolate each recipient's ref payload from the next one.
        ev.nx = 999;
    }
    public override void OnCollisionStay2D(ulong other, ref PhysicsContact2D ev, Vector2 normalToMe) { ++StayCount; }
    public override void OnCollisionExit2D(ulong other, ref PhysicsContact2D ev) { ++ExitCount; }
    public override void OnTriggerEnter2D(ulong other, ref PhysicsContact2D ev) { ++TriggerEnterCount; }
}

public static class PhysicsDriver
{
    public static int Layout()
    {
        return Marshal.SizeOf(typeof(PhysicsContact2D)) == 84 &&
            Marshal.SizeOf(typeof(PhysicsContactPoint2D)) == 12 &&
            Marshal.OffsetOf(typeof(PhysicsContact2D), "nx").ToInt32() == 33 &&
            Marshal.OffsetOf(typeof(PhysicsContact2D), "pointCount").ToInt32() == 65 &&
            Marshal.OffsetOf(typeof(PhysicsContact2D), "isSensorA").ToInt32() == 74 &&
            Marshal.OffsetOf(typeof(PhysicsContact2D), "maskB").ToInt32() == 82 ? 1 : 0;
    }
}
