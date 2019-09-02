namespace StereoKit
{
    public struct ComRender : Component<ComRender>, IComUpdate
    {
        Transform _transform;
        Mesh      _mesh;
        Material  _material;

        ComponentId _transformId;

        public ComRender(Mesh mesh, Material material, Vec3 at)
        {
            _mesh      = mesh;
            _material  = material;
            _transform = new Transform(at);
            _transformId = default(ComponentId);
        }

        public void Update()
        {
            Renderer.Add(_mesh, _material, _transform);
        }
    }
}
