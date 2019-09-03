
namespace StereoKit
{
    [ComOrderAfter(ComOrderAt.End, typeof(ComTransform))]
    public struct ComRender : Component<ComRender>, IComUpdate, IComStart
    {
        Mesh      _mesh;
        Material  _material;

        ComId<ComTransform> _transform;

        public ComRender(Mesh mesh, Material material)
        {
            _mesh      = mesh;
            _material  = material;
            _transform = default;
        }

        public void Start(EntityId entity)
        {
            _transform = entity.Find<ComTransform>();
        }

        public void Update()
        {
            Renderer.Add(_mesh, _material, _transform.Get().transform);
        }
    }
}
