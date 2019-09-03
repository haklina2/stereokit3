using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    [ComOrderAfter(ComOrderAt.End)]
    public struct ComTransform : Component<ComTransform>, IComUpdate
    {
        public Transform transform;

        public ComTransform(Vec3 at)
        {
            transform = new Transform(at);
        }

        public void Update()
        {
            transform.Update();
        }
    }
}
