using System;
using StereoKit;

struct ComScale : Component<ComScale>, IComUpdate, IComStart
{
    ComId<ComTransform> _transform;

    public void Start(EntityId entity)
    {
        _transform = entity.Find<ComTransform>();
    }

    public void Update()
    {
        ref ComTransform tr = ref _transform.Get();
        tr.Scale = Vec3.One * (float)Math.Abs(Math.Cos(StereoKitApp.Time));
    }
}
