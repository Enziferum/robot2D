using System;
using System.Runtime.InteropServices;
using robot2D;

public sealed class PhysicsReceiver : Entity
{
    public int EnterCount, StayCount, ExitCount, TriggerEnterCount, TriggerStayCount, TriggerExitCount, TriggerPhase, Errors;
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
    private void CheckTrigger(ulong other, ref PhysicsContact2D ev, PhysicsEventType expected)
    {
        if(other == ID || (ev.entityA != ID && ev.entityB != ID) || ev.type != expected ||
           ev.fixtureA == 0 || ev.fixtureB == 0 || (!ev.isSensorA && !ev.isSensorB) || ev.pointCount != 0)
            ++Errors;
    }
    public override void OnTriggerEnter2D(ulong other, ref PhysicsContact2D ev)
    {
        CheckTrigger(other, ref ev, PhysicsEventType.EnterTrigger);
        if(TriggerPhase == 1) ++Errors;
        TriggerPhase = 1;
        ++TriggerEnterCount;
    }
    public override void OnTriggerStay2D(ulong other, ref PhysicsContact2D ev)
    {
        CheckTrigger(other, ref ev, PhysicsEventType.StayTrigger);
        if(TriggerPhase != 1) ++Errors;
        ++TriggerStayCount;
    }
    public override void OnTriggerExit2D(ulong other, ref PhysicsContact2D ev)
    {
        CheckTrigger(other, ref ev, PhysicsEventType.ExitTrigger);
        if(TriggerPhase != 1) ++Errors;
        TriggerPhase = 2;
        ++TriggerExitCount;
    }
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
