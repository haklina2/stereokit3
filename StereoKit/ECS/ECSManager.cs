using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;

namespace StereoKit
{
    public delegate void WithCallback<T>(ref T item);
    internal static class ECSManager
    {
        #region Supporting Data Structures
        struct SystemInfo
        {
            public IComponentSystem system;
            public MethodInfo       addMethod;
        }
        enum SortMarker
        {
            None,
            Temp,
            Perm
        }
        #endregion

        #region Fields
        static Dictionary<int, SystemInfo> systems        = new Dictionary<int, SystemInfo>();
        static List<IComponentSystem>      systemsOrdered = new List<IComponentSystem>();
        #endregion

        #region Internal Fields
        internal static bool Start(params Assembly[] additionalAssemblies)
        {
            // Look at classes in the current DLL. TODO: take a list of assemblies for user components
            List<Assembly> assemblies  = new List<Assembly>(additionalAssemblies);
            assemblies.Insert(0,Assembly.GetExecutingAssembly());
            Type rootComponentType = typeof(Component<>).GetGenericTypeDefinition();
            foreach (Assembly assem in assemblies) { 
            foreach (Type type in assem.GetTypes())
            {
                if (type == rootComponentType)
                    continue;

                // Check if this type implements our root component type
                bool isComponent = type.GetInterfaces().Any(x =>
                  x.IsGenericType &&
                  x.GetGenericTypeDefinition() == rootComponentType);

                if (isComponent)
                {
                    // Make a system to manage that component type
                    Type systemType = typeof(ComponentSystem<>).MakeGenericType(type);
                    systems[type.GetHashCode()] = new SystemInfo {
                        system    = (IComponentSystem)Activator.CreateInstance(systemType),
                        addMethod = systemType.GetMethod("Add") 
                    };
                    
                }
            } }

            // Sort based on their dependencies
            if (!Sort())
                return false;

            foreach (var sys in systemsOrdered)
                Log.Write(LogLevel.Info, "Added component system for <~CYN>{0}<~clr>", sys.GetComType().Name);

            return true;
        }

        internal static void Shutdown()
        {
            for (int i = systemsOrdered.Count-1; i >= 0; i--)
                systemsOrdered[i].Shutdown();
        }

        internal static void Update()
        {
            Stopwatch stopWatch = new Stopwatch();
            stopWatch.Start();

            for (int i = 0, count = systemsOrdered.Count; i < count; i++)
            {
                systemsOrdered[i].Update();
            }

            stopWatch.Stop();
            //Console.WriteLine("Update " + stopWatch.Elapsed.TotalMilliseconds + "ms");
        }

        internal static ComId Add<T>(EntityId entity, ref T item)
        {
            int hash = typeof(T).GetHashCode();
            SystemInfo info = systems[hash];
            return new ComId { 
                system = info.system, 
                index  = (int)info.addMethod.Invoke(info.system, new object[]{ entity, item })
            };
        }

        internal static IComponentSystem GetSystem<T>()
        {
            return systems[typeof(T).GetHashCode()].system;
        }
        #endregion

        #region Dependency Based Component System Sorting
        private static bool Sort()
        {
            // Topological sort, Depth-first algorithm:
            // https://en.wikipedia.org/wiki/Topological_sorting

            // Create a list of systems for us to sort! Include start and end items so we know where to anchor (ComOrderAt) around.
            List<IComponentSystem> startSystems = systems.Select(s=>s.Value.system).ToList();
            List<Type>             start        = systems.Select(s=>s.Value.system.GetComType()).ToList();
            start.Insert(0, null); // ComOrderAt.Start
            start.Add   (null);    // ComOrderAt.End
            int startId = 0;
            int endId   = start.Count-1;

            // Build a list of dependency edges between components
            List<List<int>> edges = new List<List<int>>(start.Count);
            for (int i = 0; i < start.Count; i++)
                edges.Add(new List<int>());
            
            for (int i = 0; i < start.Count; i++)
            {
                if (start[i] == null)
                    continue;

                ComOrderAfter  after  = Attribute.GetCustomAttribute(start[i], typeof(ComOrderAfter )) as ComOrderAfter;
                ComOrderBefore before = Attribute.GetCustomAttribute(start[i], typeof(ComOrderBefore)) as ComOrderBefore;

                // Dependencies this component has on others
                if (before != null)
                { 
                    if (before.Anchor != ComOrderAt.None)
                        edges[i].Add(before.Anchor == ComOrderAt.Start ? startId : endId);
                    for (int t = 0; t < before.ComponentTypes.Length; t++)
                    {
                        int id = start.IndexOf(before.ComponentTypes[t]);
                        if (!edges[i].Contains(id))
                            edges[i].Add(id);
                    }
                }

                // Dependencies other components have on this
                if (after != null)
                { 
                    if (after.Anchor != ComOrderAt.None)
                        edges[after.Anchor == ComOrderAt.Start ? startId : endId].Add(i);
                    for (int t = 0; t < after.ComponentTypes.Length; t++)
                    {
                        int id = start.IndexOf(after.ComponentTypes[t]);
                        if (!edges[id].Contains(i))
                            edges[id].Add(i);
                    }
                }

                // If none specified, just make sure we're after start, and before end
                if (after == null && before == null)
                { 
                    edges[i].Add(endId);
                    edges[startId].Add(i);
                }
            }

            // Now sort!
            int[]     outOrder   = new int[start.Count];
            SortMarker[] marks      = new SortMarker[start.Count];
            int       sortedCurr = start.Count - 1;

            while (sortedCurr > 0)
            {
                for (int i = 0, count = start.Count; i < count; i++)
                {
                    if (marks[i] != 0)
                        continue;
                    int result = SortVisit(edges, i, marks, ref sortedCurr, outOrder);
                    // If we found a cyclic dependency, ditch out!
                    if (result != 0)
                    {
                        Log.Write(LogLevel.Error, "Cyclic dependency detected at {0}! Please check your ComOrderBefore/After attributes to ensure components don't mutually depend on each other!", start[i].Name);
                        return false;
                    }
                }
            }

            // Reorder the systems
            for (int i = outOrder.Length-1; i >= 0; i--)
            {
                if (start[outOrder[i]] != null) // Check for an End/Start anchor
                    systemsOrdered.Add(startSystems[outOrder[i]-1]);
            }

            return true;
        }
        private static int SortVisit(List<List<int>> dependencies, int index, SortMarker[] marks, ref int sorted_curr, int[] out_order)
        {
            if (marks[index] == SortMarker.Perm) return 0;
            if (marks[index] == SortMarker.Temp) return index;
            marks[index] = SortMarker.Temp;
            for (int i = 0; i < dependencies.Count; i++)
            {
                for (int d = 0; d < dependencies[i].Count; d++)
                {
                    if (dependencies[i][d] == index)
                    {
                        int result = SortVisit(dependencies, i, marks, ref sorted_curr, out_order);
                        if (result != 0)
                            return result;
                    }
                }
            }
            marks    [index] = SortMarker.Perm;
            out_order[sorted_curr] = index;
            sorted_curr -= 1;
            return 0;
        }
        #endregion
    }
}
