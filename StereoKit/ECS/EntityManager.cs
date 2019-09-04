using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{ 
    internal static class EntityManager
    {
        internal struct EntityInfo
        {
            public Entity entity;
            public ushort slotId;
            public bool   used;
        }

        internal static EntityInfo[] list       = new EntityInfo[1];
        private  static int          _firstOpen = 0;
        
        static EntityManager()
        {
            Create("Invalid");
        }

        public   static EntityId Create(string name)
        {
            // Find an empty slot
            int slot = -1;
            for (int i = _firstOpen; i < list.Length; i++)
            {
                if (!list[i].used)
                {
                    slot = i;
                    break;
                }
            }
            // Or make a new, empty slot
            if (slot == -1)
            {
                slot = list.Length;
                Array.Resize(ref list, list.Length * 2);
            }
            _firstOpen = slot + 1;

            // Fill that slot with a new entity
            EntityId id = new EntityId {
                index  = slot,
                slotId = list[slot].slotId
            };
            list[slot] = new EntityInfo {
                entity = new Entity(id, name),
                slotId = list[slot].slotId,
                used   = true,
            };

            return id;
        }
        public   static void Remove(EntityId id)
        {
            if (id.slotId != list[id.index].slotId)
                Log.Write(LogLevel.Error, "Trying to remove an entity that's already been removed!");

            // TODO: Make sure all components get a destroy message
            for (int i = 0; i < list[id.index].entity._components.Length; i++)
            {
                throw new System.NotImplementedException();
            }

            // Delete this entity
            list[id.index].used    = false;
            list[id.index].slotId += 1; // Next slot Id, increment now so any access errors surface earlier.
            list[id.index].entity  = default;

            // If it's the first slot, keep track of it
            if (_firstOpen > id.index)
                _firstOpen = id.index;
        }
    }
}
