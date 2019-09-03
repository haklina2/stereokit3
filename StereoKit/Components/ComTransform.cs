using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    public struct ComTransform : Component<ComTransform>
    {
        public Transform transform;

        public ComTransform(Vec3 at)
        {
            transform = new Transform(at);
        }
    }
}
