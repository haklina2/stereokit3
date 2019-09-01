namespace StereoKit
{
    public struct ComRender : Component<ComRender>, IComStart, IComUpdate
    {
        Transform _transform;
        Model     _model;
        string    _modelName;

        public ComRender(string modelName)
        {
            _modelName = modelName;
            _model     = null;
            _transform = new Transform();
        }

        public void Start()
        {
            _model = new Model(_modelName);
        }

        public void Update()
        {
            Renderer.Add(_model, _transform);
        }
    }
}
