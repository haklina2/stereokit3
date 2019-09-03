
namespace StereoKit
{
    [ComOrderAfter(ComOrderAt.End)]
    public struct ComTransform : Component<ComTransform>, IComUpdate
    {
        public Transform transform;
        public Vec3 Position { get{ return transform.Position;  } set{ transform.Position  = value; } }
        public Vec3 Scale    { get { return transform.Scale;    } set { transform.Scale    = value; } }
        public Quat Rotation { get { return transform.Rotation; } set { transform.Rotation = value; } }

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
