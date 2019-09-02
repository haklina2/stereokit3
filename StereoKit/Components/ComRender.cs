namespace StereoKit
{
    public struct ComRender : Component<ComRender>, IComStart, IComUpdate
    {
        Transform _transform;
        Model     _model;
        string    _modelName;

        ComponentId _transformId;

        public ComRender(string modelName)
        {
            _modelName = modelName;
            _model     = null;
            _transform = new Transform();
            _transformId = default(ComponentId);
        }
        public ComRender(Model  model)
        {
            _modelName = null;
            _model     = model;
            _transform = new Transform();
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
