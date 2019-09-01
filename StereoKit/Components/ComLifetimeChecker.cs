namespace StereoKit
{
    public struct ComLifetimeChecker : Component<ComLifetimeChecker>, IComStart, IComEnabled, IComUpdate, IComDisabled, IComDestroy
    {
        bool _updateLogged;

        public void Start()
        {
            Log.Write(LogLevel.Info, "Start");
        }
        public void Enabled()
        {
            Log.Write(LogLevel.Info, "Enabled");
        }
        public void Update()
        {
            Log.Write(LogLevel.Info, "Update");
        }
        public void Disabled()
        {
            Log.Write(LogLevel.Info, "Disabled");
        }
        public void Destroy()
        {
            Log.Write(LogLevel.Info, "Destroy");
        }
    }
}
