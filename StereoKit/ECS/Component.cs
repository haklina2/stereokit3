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

    public struct ComId
    {
        internal int              index;
        internal IComponentSystem system;
        internal ushort           slotId;
    }

    public struct ComId<T> where T : struct, Component<T>
    { 
        ComId _id;

        public bool Enabled { set { SetEnabled(value);      } }
        public bool Valid   { get { return _id.system != null; } }

        public ComId(ComId id)
        {
            _id = id;
        }
        
        public void With(WithCallback<T> with)
        {
            ((ComponentSystem<T>)_id.system).With(_id, with);
        }
        public T Read()
        {
            return ((ComponentSystem<T>)_id.system).Read(_id);
        }
        public void SetEnabled(bool enabled)
        {
            _id.system.SetEnabled(_id, enabled);
        }
    }

    public enum ComOrderAt
    {
        None,
        End,
        Start
    }

    [AttributeUsage(AttributeTargets.Struct)]
    public class ComOrderBefore : Attribute
    {
        public Type[]     ComponentTypes { private set; get; }
        public ComOrderAt Anchor         { private set; get; }

        public ComOrderBefore(params Type[] componentTypes)
        {
            ComponentTypes = componentTypes;
            Anchor         = ComOrderAt.None;
        }
        public ComOrderBefore(ComOrderAt anchor, params Type[] componentTypes)
        {
            ComponentTypes = componentTypes;
            Anchor         = anchor;
        }
    }

    [AttributeUsage(AttributeTargets.Struct)]
    public class ComOrderAfter : Attribute
    {
        public Type[]     ComponentTypes { private set; get; }
        public ComOrderAt Anchor         { private set; get; }

        public ComOrderAfter(params Type[] componentTypes)
        {
            ComponentTypes = componentTypes;
            Anchor         = ComOrderAt.None;
        }
        public ComOrderAfter(ComOrderAt anchor, params Type[] componentTypes)
        {
            ComponentTypes = componentTypes;
            Anchor         = anchor;
        }
    }
}
