
namespace StereoKit
{
    public interface IComUpdate       { void Update(); }
    public interface IComStart        { void Start(ComId self, EntityId entity); }
    public interface IComDestroy      { void Destroy(); }
    public interface IComEnabled      { void Enabled(); }
    public interface IComDisabled     { void Disabled(); }
}