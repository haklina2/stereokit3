using System;
using StereoKit;

struct ComScale : Component<ComScale>, IComUpdate, IComStart
{
    ComId<ComTransform> _transform;
    static int id =0;
    float offset;

    public void Start(ComId self, EntityId entity)
    {
        _transform = entity.Find<ComTransform>();
        id++;
        offset = id*0.13f;
    }

    public void Update()
    {
        ref ComTransform tr = ref _transform.Get();
        tr.Scale = Vec3.One * (float)Math.Abs(Math.Cos(StereoKitApp.Time+offset));
    }
}
