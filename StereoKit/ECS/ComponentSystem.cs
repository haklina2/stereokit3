using System;
using System.Collections.Generic;

namespace StereoKit
{ 
    interface IComponentSystem { 
        void Update    (); 
        void SetEnabled(ComId id, bool enabled);
        bool GetEnabled(ComId id);
        void Remove    (ComId id);
        void Shutdown  ();
        Type GetComType();
    }

    internal class ComponentSystem<T> : IComponentSystem where T : struct, Component<T>
    {
        #region Supporting data types
        internal enum Lifetime
        {
            Empty,
            Uninitialized,
            Ready,
            Destroy
        }

        internal struct SlotInfo
        {
            public Lifetime life;
            public int      current;
            public bool     enabled;
        }

        struct StartInfo
        {
            public int index;
            public EntityId entity;
        }
        #endregion

        #region Fields
        internal T[]         _components  = new T[1];
        internal SlotInfo[]  _info        = new SlotInfo[1];
        List<StartInfo> _needStart   = new List<StartInfo>();
        List<int>       _needDestroy = new List<int>();
        int             _firstOpen   = 0;

        bool _hasUpdate;
        bool _hasStart;
        bool _hasDestroy;
        bool _hasEnabled;
        bool _hasDisabled;
        #endregion

        #region Constructors
        public ComponentSystem()
        {
            Type type = typeof(T);

            _hasUpdate   = typeof(IComUpdate  ).IsAssignableFrom(type);
            _hasStart    = typeof(IComStart   ).IsAssignableFrom(type);
            _hasDestroy  = typeof(IComDestroy ).IsAssignableFrom(type);
            _hasEnabled  = typeof(IComEnabled ).IsAssignableFrom(type);
            _hasDisabled = typeof(IComDisabled).IsAssignableFrom(type);
        }
        #endregion

        #region Public Methods
        public void Shutdown()
        {
            for (int i = 0, count = _components.Length; i < count; i++) { 
                if (_info[i].life != Lifetime.Empty)
                    DestroyComponent(i);
            }
        }

        public void Update()
        {
            // Check for items that need a Start event
            if (_needStart.Count > 0)
            {
                for (int i = 0, count = _needStart.Count; i < count; i++)
                    InitializeComponent(_needStart[i]);
                _needStart.Clear();
            }

            // Check for items that need an Update event
            if (_hasUpdate)
            {
                for (int i = 0, count = _components.Length; i < count; i++)
                {
                    if (_info[i].life == Lifetime.Ready && _info[i].enabled)
                        UpdateComponent(i);
                }
            }

            // Check for items that need destroyed
            if (_needDestroy.Count > 0)
            {
                for (int i = 0, count = _needDestroy.Count; i < count; i++)
                    DestroyComponent(i);
                _needDestroy.Clear();
            }
        }

        public Type GetComType()
        {
            return typeof(T);
        }

        public int Add(EntityId entity, ref T item)
        {
            // Find an open slot in the list!
            int slot = -1;
            for (int i = _firstOpen, count = _components.Length; i < count; i++)
            {
                if (_info[i].life == Lifetime.Empty)
                {
                    slot = i;
                    _components[i] = item;
                    break;
                }
            }

            if (slot == -1)
            { // No slot found, add space, and select a new one
                slot = _components.Length;
                Array.Resize(ref _components, _components.Length*2);
                Array.Resize(ref _info,       _components.Length);
            } 

            // fill in the slot we selected with information for the new component!
            _info[slot] = new SlotInfo { 
                life    = Lifetime.Uninitialized, 
                current = _info[slot].current, // DestroyComponent already increments this, to raise issues earlier
                enabled = true };
            _components[slot] = item;
            
            
            _needStart.Add(new StartInfo {index = slot, entity = entity });
            _firstOpen = slot+1;
            return slot;
        }

        public void Remove(ComId id)
        {
            // If the slot id doesn't match, then we're trying to do something to a component that
            // used to be in this slot, but is no longer.
            if (_info[id.index].current != id.slotId) { 
                Log.Write(LogLevel.Warning, "Trying to remove a component that was already destroyed!");
                return;
            }
            // Mark it for destroy
            _info[id.index] = new SlotInfo { life = Lifetime.Destroy, current = id.slotId, enabled = false };
            _needDestroy.Add(id.index);
        }
        
        public void SetEnabled(ComId id, bool enabled)
        {
            // If the slot id doesn't match, then we're trying to do something to a component that
            // used to be in this slot, but is no longer.
            if (_info[id.index].current != id.slotId)
            {
                Log.Write(LogLevel.Warning, "Trying to enable a component that was already destroyed!");
                return;
            }

            // Don't duplicate enable/disable
            if (_info[id.index].enabled == enabled)
                return;

            if (enabled  && _hasEnabled && _info[id.index].life == Lifetime.Ready) {
                IComEnabled com = (IComEnabled)_components[id.index];
                com.Enabled();
                _components[id.index] = (T)com;
            }

            if (!enabled && _hasDisabled && _info[id.index].life == Lifetime.Ready) {
                IComDisabled com = (IComDisabled)_components[id.index];
                com.Disabled();
                _components[id.index] = (T)com;
            }

            _info[id.index] = new SlotInfo
            {
                enabled = enabled,
                life    = _info[id.index].life,
                current = _info[id.index].current
            };
        }

        public bool GetEnabled(ComId id)
        {
            if (_info[id.index].current != id.slotId)
            {
                Log.Write(LogLevel.Warning, "Trying to GetEnabled a component that was already destroyed!");
                return false;
            }

            return _info[id.index].enabled;
        }
        #endregion

        #region Private Methods
        void InitializeComponent(StartInfo start)
        {
            if (_hasStart) {
                IComStart com = (IComStart)_components[start.index];
                com.Start(start.entity);
                _components[start.index] = (T)com;
            }

            if (_hasEnabled && _info[start.index].enabled) { 
                IComEnabled com = (IComEnabled)_components[start.index];
                com.Enabled();
                _components[start.index] = (T)com;
            }

            _info[start.index] = new SlotInfo { 
                life    = Lifetime.Ready, 
                current = _info[start.index].current,
                enabled = _info[start.index].enabled };
        }

        void DestroyComponent(int index)
        {
            if (_hasDisabled && _info[index].enabled) { 
                IComDisabled com = (IComDisabled)_components[index];
                com.Disabled();
                _components[index] = (T)com;
            }

            if (_hasDestroy) { 
                IComDestroy com = (IComDestroy)_components[index];
                com.Destroy();
                _components[index] = (T)com;
            }

            // Clear the data, and mark it as destroyed
            _components[index] = default(T);
            _info      [index] = new SlotInfo {
                life    = Lifetime.Empty,
                current = _info[index].current+1,
                enabled = false };

            // Mark this slot as the first open slot, if it's in front of the last one
            if (index < _firstOpen)
                _firstOpen = index;
        }

        void UpdateComponent(int index)
        {
            IComUpdate update = (IComUpdate)_components[index];
            update.Update();
            _components[index] = (T)update;
        }
        #endregion
    }
}
