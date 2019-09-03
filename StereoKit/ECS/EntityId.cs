
namespace StereoKit
{
    public struct EntityId
    {
        internal int    index;
        internal ushort slotId;

        public ComId<T> Add<T>(T component) where T : struct, Component<T>
        {
            Entity.EntityInfo e = Entity._list[index];
            ComId<T> result = e.entity.Add(component);
            Entity._list[index] = e;
            return result;
        }
        public void Remove<T>() where T : struct, Component<T>
        {
            Entity.EntityInfo e = Entity._list[index];
            e.entity.Remove<T>();
            Entity._list[index] = e;
        }
        public ComId<T> Get<T>() where T : struct, Component<T>
        {
            return Entity._list[index].entity.Get<T>();
        }
    }
}
