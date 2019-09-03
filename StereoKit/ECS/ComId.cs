
namespace StereoKit
{
    /// <summary>
    /// A non-generic ComId for internal use. Doesn't need to know what type it's
    /// dealing with!
    /// </summary>
    internal struct ComId
    {
        internal int              index;
        internal IComponentSystem system;
        internal ushort           slotId;
    }

    /// <summary> An ComId is a smart index to a Component struct. Since you cannot
    /// store a reference to a struct directly, this Id simulates some of that
    /// reference style behavior. All calls made to this Id get passed on to 
    /// the Component struct in the system's array! </summary>
    public struct ComId<T> where T : struct, Component<T>
    { 
        ComId _id;

        /// <summary> Does this Component receive events from the system? Fires Enabled/Disabled
        /// events when changed, and includes a guard to prevent redundant assignments. </summary>
        public bool Enabled { set { SetEnabled(value); } get { return GetEnabled(); } }
        /// <summary> Does this Id point to a valid Component? Since a struct cannot be null, use 
        /// this to check for failure on calls that return a Component, or to check if the 
        /// Component itself has been deleted.</summary>
        public bool Valid   { get { return _id.system != null; } }

        internal ComId(ComId id)
        {
            _id = id;
        }
        
        /// <summary> Gets a reference directly to the Component struct so you can read and modify it!</summary>
        public ref T Get()
        {
            ComponentSystem<T> sys = (ComponentSystem<T>)_id.system;
            if (sys._info[_id.index].current != _id.slotId) {
                throw new System.Exception("Trying to Get on deleted Component!");
            }
            return ref sys._components[_id.index];
        }

        /// <summary> Fires Enabled/Disabled events when changed, and includes a guard to 
        /// prevent redundant assignments. The Enabled property calls this function.</summary>
        public void SetEnabled(bool enabled)
        {
            _id.system.SetEnabled(_id, enabled);
        }

        /// <summary> Does this Component receive events from the system? The Enabled 
        /// property calls this function</summary>
        public bool GetEnabled()
        {
            return _id.system.GetEnabled(_id);
        }
    }
}
