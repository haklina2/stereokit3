using System;
using System.Collections.Generic;

namespace StereoKit
{
    public struct Entity
    {
        internal struct EntityInfo
        {
            public Entity entity;
            public ushort slotId;
            public bool   used;
        }

        internal static EntityInfo[] _list      = new EntityInfo[1];
        private  static int          _firstOpen = 0;
        public   static EntityId Create(string name)
        {
            // Find an empty slot
            int slot = -1;
            for (int i = _firstOpen; i < _list.Length; i++)
            {
                if (!_list[i].used)
                {
                    slot = i;
                    break;
                }
            }
            // Or make a new, empty slot
            if (slot == -1)
            {
                slot = _list.Length;
                Array.Resize(ref _list, _list.Length * 2);
            }
            _firstOpen = slot + 1;

            // Fill that slot with a new entity
            EntityId id = new EntityId {
                index  = slot,
                slotId = _list[slot].slotId
            };
            _list[slot] = new EntityInfo {
                entity = new Entity(id, name),
                slotId = _list[slot].slotId,
                used   = true,
            };

            return id;
        }
        public   static void Remove(EntityId id)
        {
            if (id.slotId != _list[id.index].slotId)
                Log.Write(LogLevel.Error, "Trying to remove an entity that's already been removed!");

            // TODO: Make sure all components get a destroy message
            for (int i = 0; i < _list[id.index].entity._components.Length; i++)
            {
                throw new System.NotImplementedException();
            }

            // Delete this entity
            _list[id.index].used    = false;
            _list[id.index].slotId += 1; // Next slot Id, increment now so any access errors surface earlier.
            _list[id.index].entity  = default;

            // If it's the first slot, keep track of it
            if (_firstOpen > id.index)
                _firstOpen = id.index;
        }

        EntityId _id;
        string   _name;
        ComId[]  _components;

        private Entity(EntityId id, string name)
        {
            _id         = id;
            _name       = name;
            _components = null;
        }

        public ComId<T> Add<T>(T component) where T : struct, Component<T>
        {
            int count = _components == null ? 0 : _components.Length;
            Array.Resize<ComId>(ref _components, count+1);
            
            _components[count] = ECSManager.Add(_id, ref component);
            return new ComId<T>(_components[count]);
        }

        public void Remove<T>() where T : struct, Component<T>
        {
            throw new NotImplementedException();
        }

        public ComId<T> Get<T>() where T : struct, Component<T>
        {
            IComponentSystem system = ECSManager.GetSystem<T>();
            for (int i = 0; i < _components.Length; i++)
            {
                if (_components[i].system == system)
                    return new ComId<T>(_components[i]);
            }
            return default;
        }
    }
}
