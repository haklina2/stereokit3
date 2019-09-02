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

    public struct ComId<T> where T : struct, Component<T>
    { 
        ComponentId _id;

        public bool Enabled { set { SetEnabled(value); } }
        public bool Valid { get { return _id.system != 0; } }

        public ComId(ComponentId id)
        {
            _id = id;
        }
        
        public void With(WithCallback<T> with)
        {
            ECSManager.With(_id, with);
        }
        public void SetEnabled(bool enabled)
        {
            ECSManager.SetEnabled(_id, enabled);
        }
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
