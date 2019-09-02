namespace StereoKit
{
    public struct ComRender : Component<ComRender>, IComStart, IComUpdate
    {
        Transform _transform;
        Model     _model;
        string    _modelName;

        ComponentId _transformId;

        public ComRender(string modelName, Vec3 at)
        {
            _modelName = modelName;
            _model     = null;
            _transform = new Transform(at);
            _transformId = default(ComponentId);
        }
        public ComRender(Model  model, Vec3 at)
        {
            _modelName = null;
            _model     = model;
            _transform = new Transform(at);
            _transformId = default(ComponentId);
        }

        public void Start()
        {
            if (_modelName != null)
                _model = new Model(_modelName);
        }

        public void Update()
        {
            Renderer.Add(_model, _transform);
        }
    }
}
