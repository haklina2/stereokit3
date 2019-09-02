using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    public class Entity
    {
        string        _name;
        ComponentId[] _components = null;

        public Entity(string name)
        {
            _name = name;
        }

        public ComId<T> Add<T>(T component) where T : struct, Component<T>
        {
            int count = _components == null ? 0 : _components.Length;
            Array.Resize<ComponentId>(ref _components, count+1);
            
            _components[count] = ECSManager.Add(ref component);
            return new ComId<T>(_components[count]);
        }
        public void Remove<T>() where T : struct, Component<T>
        {
            
        }
        public ComId<T> Get<T>() where T : struct, Component<T>
        {
            int hash = typeof(T).GetHashCode();
            for (int i = 0; i < _components.Length; i++)
            {
                if (_components[i].system == hash)
                    return new ComId<T>(_components[i]);
            }
            return default;
        }
    }
}
