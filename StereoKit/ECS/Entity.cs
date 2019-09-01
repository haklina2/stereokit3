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

        public void Add<T>(T component)
        {
            int count = _components == null ? 0 : _components.Length;
            Array.Resize<ComponentId>(ref _components, count+1);
            
            _components[count] = ECSManager.Add(ref component);
        }
        public void Remove<T>()
        {

        }
    }
}
