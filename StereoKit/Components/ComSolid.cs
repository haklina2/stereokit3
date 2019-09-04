
namespace StereoKit
{
    [ComOrderBefore(typeof(ComTransform))]
    public struct ComSolid : Component<ComSolid>, IComStart, IComUpdate
    {
        Solid _solid;
        ComId<ComTransform> _transform;

        public void Start(ComId self, EntityId entity)
        {
            _transform = entity.Find<ComTransform>();
            if (_transform.Valid)
                _solid = new Solid(Vec3.Zero, Quat.Identity);
            else
                self.SetEnabled(false);
        }

        public void Update()
        {
            _solid.GetTransform(ref _transform.Get().transform);
        }
    }
}
