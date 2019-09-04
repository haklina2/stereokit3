using System;

namespace StereoKit
{
    public enum ComOrderAt
    {
        None,
        End,
        Start
    }

    [AttributeUsage(AttributeTargets.Struct)]
    public class ComOrderBefore : Attribute
    {
        #region Properties
        public Type[]     ComponentTypes { private set; get; }
        public ComOrderAt Anchor         { private set; get; }
        #endregion

        #region Constructors
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
        #endregion
    }

    [AttributeUsage(AttributeTargets.Struct)]
    public class ComOrderAfter : Attribute
    {
        #region Properties
        public Type[]     ComponentTypes { private set; get; }
        public ComOrderAt Anchor         { private set; get; }
        #endregion

        #region Constructors
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
        #endregion
    }

    [AttributeUsage(AttributeTargets.Struct)]
    public class ComNoThread : Attribute
    {
    }
}
