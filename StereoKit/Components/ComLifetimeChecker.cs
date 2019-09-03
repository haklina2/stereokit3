
namespace StereoKit
{
    [ComOrderBefore(ComOrderAt.Start)]
    public struct ComLifetimeChecker : Component<ComLifetimeChecker>, IComStart, IComEnabled, IComUpdate, IComDisabled, IComDestroy
    {
        bool _updateLogged;
        int _lastCounter;
        public int counter;

        public void Start(EntityId entity)
        {
            //Log.Write(LogLevel.Info, "Start");
        }
        public void Enabled()
        {
            //Log.Write(LogLevel.Info, "Enabled");
        }
        public void Update()
        {
            if (!_updateLogged) { 
                _updateLogged = true;
                //Log.Write(LogLevel.Info, "Update");
            }

            //counter += 1;
            if (counter != _lastCounter)
            {
                Log.Write(LogLevel.Info, "Counter changed to: {0}", counter);
                _lastCounter = counter;
            }
        }
        public void Disabled()
        {
            //Log.Write(LogLevel.Info, "Disabled");
        }
        public void Destroy()
        {
            //Log.Write(LogLevel.Info, "Destroy");
        }
    }
}
