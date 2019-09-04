using System.Collections.Generic;

namespace StereoKit
{
    [ComOrderAfter(ComOrderAt.End)]
    public struct ComTransform : Component<ComTransform>, IComUpdate, IComStart
    {
        private ComId       _parentTransform;
        private List<ComId> _childTransforms;
        public  Transform   transform;

        public Vec3 Position { get { return transform.Position; } set { transform.Position = value; } }
        public Vec3 Scale    { get { return transform.Scale;    } set { transform.Scale    = value; } }
        public Quat Rotation { get { return transform.Rotation; } set { transform.Rotation = value; } }

        public ComTransform(Vec3 at)
        {
            transform = new Transform(at);
            _parentTransform = default;
            _childTransforms = null;
        }

        public void Start(ComId self, EntityId entity)
        {
            EntityId curr = entity.Parent;
            while (curr.Valid)
            {
                _parentTransform = entity.Parent.Find<ComTransform>();
                if (_parentTransform.Valid)
                    break;
                curr = curr.Parent;
            }  

            if (_parentTransform.Valid)
            {
                ref ComTransform parentTr = ref _parentTransform.Get<ComTransform>();
                if (parentTr._childTransforms == null)
                    parentTr._childTransforms = new List<ComId>();
                parentTr._childTransforms.Add(self);
            }
        }

        public void Update()
        {
            // If this has no parent, then it's a root node we can propogate down from,
            // otherwise, it's a child node that'll get updated from a parent.
            if (!_parentTransform.Valid) { 
                UpdateNode(null, false);
            }
        }
        void UpdateNode(Transform parent, bool parentDirty)
        {
            // Update this node
            bool dirty = parentDirty || transform.Dirty;
            if (dirty) { 
                if (parent != null)
                    transform.ApplyParent(parent);
                else
                    transform.Update();
            }
               
            // Update all child nodes
            if (_childTransforms != null)
            {
                for (int i = 0, c = _childTransforms.Count; i < c; i++)
                {
                    _childTransforms[i].Get<ComTransform>().UpdateNode(transform, dirty);
                }
            }
        }
    }
}
