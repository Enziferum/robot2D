namespace robot2D
{
    public class SceneManager
    {
        public static bool LoadScene(string name)
        {
            return InternalCalls.SceneManager_LoadScene(name);
        }
        
        public static void LoadSceneAsync(string name)
        {
            InternalCalls.SceneManager_LoadSceneAsync(name);
        }
    }
}