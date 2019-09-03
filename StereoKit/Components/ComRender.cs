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
            _transform = entity.Get<ComTransform>();
        }

        public void Update()
        {
            Transform tr = _transform.Read().transform;
            Renderer.Add(_mesh, _material, tr);
        }
    }
}
