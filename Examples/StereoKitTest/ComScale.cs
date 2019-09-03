using System;
using StereoKit;

struct ComScale : Component<ComScale>, IComUpdate, IComStart
{
    ComId<ComTransform> _transform;

    public void Start(EntityId entity)
    {
        _transform = entity.Get<ComTransform>();
    }

    public void Update()
    {
        _transform.With((ref ComTransform tr) => {
            tr.transform.Scale = Vec3.One * (float)Math.Abs(Math.Cos(StereoKitApp.Time));
        });
    }
}
