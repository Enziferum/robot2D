namespace robot2D
{
    public enum MouseButton: int
    {
        Left = 0,
        Right = 1,
        Middle = 2
    }
    
    public class Input
    {
        public static bool IsKeyDown(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyDown(keycode);
        }
        
        public static bool IsMousePressed(MouseButton mouseButton)
        {
            return InternalCalls.Input_IsMousePressed(mouseButton);
        }

        public static Vector2 GetMousePosition()
        {
            InternalCalls.Input_GetMousePosition(out Vector2 position);
            return position;
        }

        public static void SetMousePosition(Vector2 position)
        {
            InternalCalls.Input_SetMousePosition(ref position);
        }
       
    }
}