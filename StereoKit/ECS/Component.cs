using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    public interface Component<T> where T : struct
    {
        // Maybe when default interface methods work?
        /*delegate void WithCallback(ref T item);
        void With(WithCallback with)
        {
            with(ref this);
        }*/
    }

    public struct ComponentId
    {
        public int system;
        public int index;
        public int slotId;
    }

    struct TransformCom : Component<TransformCom>
    {
        Transform _transform;
        Model     _model;

        public TransformCom(string modelName)
        {
            _model = new Model(modelName);
            _transform = new Transform();
        }

        public void Update()
        {
            Renderer.Add(_model, _transform);
        }
    }
}
