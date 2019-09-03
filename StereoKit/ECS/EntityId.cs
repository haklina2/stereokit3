
namespace StereoKit
{
    /// <summary> An EntityId is a smart index to an Entity struct. Since you 
    /// cannot store a reference to a struct directly, this Id simulates some 
    /// of that reference style behavior. All calls made to this Id get passed 
    /// on to the Entity struct in the system's array! </summary>
    public struct EntityId
    {
        /// <summary> Index in the entity array found in ECSManager.</summary>
        internal int    index;
        /// <summary> Since empty slots in the entity array will be re-used, this 
        /// Id distinguishes this Id from that of an old, deleted Entity with a 
        /// different slot Id. </summary>
        internal ushort slotId;

        /// <summary> Adds a component to the Entity, and returns a smart Id to the 
        /// new component. A Start event will be queued up for the component before
        /// the next Update. </summary>
        /// <typeparam name="T">Any StereoKit Component struct.</typeparam>
        /// <param name="component">A Component to be added into the system, and attached to this Entity.</param>
        /// <returns>A smart Id pointing to the newly added Component.</returns>
        public ComId<T> Add<T>(T component) where T : struct, Component<T>
        {
            Entity.EntityInfo e = Entity._list[index];
            ComId<T> result = e.entity.Add(component);
            Entity._list[index] = e;
            return result;
        }

        /// <summary> Removes the first instance of a 'T' Component attached to this Entity. </summary>
        /// <typeparam name="T">Any StereoKit Component struct.</typeparam>
        public void Remove<T>() where T : struct, Component<T>
        {
            Entity.EntityInfo e = Entity._list[index];
            e.entity.Remove<T>();
            Entity._list[index] = e;
        }

        /// <summary> Searches for the first instance of a 'T' component associated with
        /// this Entity, and returns a smart index to it. If no component is found, then
        /// the return value's IsValid property will be false. </summary>
        /// <typeparam name="T">Any StereoKit Component struct.</typeparam>
        /// <returns>Smart index to the found matching Component. If no component is found, the result's IsValid will be false.</returns>
        public ComId<T> Find<T>() where T : struct, Component<T>
        {
            return Entity._list[index].entity.Find<T>();
        }
    }
}
