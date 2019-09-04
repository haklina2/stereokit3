using System;
using System.Collections.Generic;

namespace StereoKit
{
    public struct Entity
    {
        private  EntityId       _id;
        internal EntityId       _parent;
        internal List<EntityId> _children;
        internal string         _name;
        internal ComId[]        _components;

        internal Entity(EntityId id, string name)
        {
            _id         = id;
            _name       = name;
            _components = null;
            _parent     = default;
            _children   = null;
        }
        public static EntityId Create(string name)
        {
            return EntityManager.Create(name);
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

        public ComId<T> Find<T>() where T : struct, Component<T>
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
