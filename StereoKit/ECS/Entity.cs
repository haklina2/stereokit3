using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    public struct EntityId
    {
        internal int    index;
        internal ushort slotId;

        public ComId<T> Add<T>(T component) where T : struct, Component<T>
        {
            Entity.EntityInfo e = Entity._list[index];
            ComId<T> result = e.entity.Add(component);
            Entity._list[index] = e;
            return result;
        }
        public void Remove<T>() where T : struct, Component<T>
        {
            Entity.EntityInfo e = Entity._list[index];
            e.entity.Remove<T>();
            Entity._list[index] = e;
        }
        public ComId<T> Get<T>() where T : struct, Component<T>
        {
            return Entity._list[index].entity.Get<T>();
        }
    }

    public struct Entity
    {
        internal struct EntityInfo
        {
            public Entity entity;
            public ushort slotId;
        }
        internal static List<EntityInfo> _list = new List<EntityInfo>();
        public static EntityId Create(string name)
        {
            EntityId id = new EntityId {
                index  = _list.Count,
                slotId = 0,
            };
            _list.Add(new EntityInfo {
                entity = new Entity(id, name),
                slotId = 0,
            });

            return id;
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
