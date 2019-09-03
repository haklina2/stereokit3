using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;

namespace StereoKit
{
    public delegate void WithCallback<T>(ref T item);
    public class ECSManager
    {
        struct SystemInfo
        {
            public IComponentSystem system;
            public MethodInfo       addMethod;
        }
        
        static Dictionary<int, SystemInfo> systems        = new Dictionary<int, SystemInfo>();
        static List<IComponentSystem>      systemsOrdered = new List<IComponentSystem>();
        
        public static void Start()
        {
            // Look at classes in the current DLL. TODO: take a list of assemblies for user components
            Assembly assembly          = Assembly.GetExecutingAssembly();
            Type     rootComponentType = typeof(Component<>).GetGenericTypeDefinition();
            foreach (Type type in assembly.GetTypes())
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
            }

            // Sort based on their dependencies
            Sort();

            foreach (var sys in systemsOrdered)
                Log.Write(LogLevel.Info, "Added component system for <~CYN>{0}<~clr>", sys.GetComType().Name);
        }

        public static void Shutdown()
        {
            for (int i = systemsOrdered.Count-1; i >= 0; i--)
                systemsOrdered[i].Shutdown();
        }

        public static void Update()
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

        public static ComId Add<T>(EntityId entity, ref T item)
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

        enum TopSort
        {
            None,
            Temp,
            Perm
        }
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
            }

            // Now sort!
            int[]     outOrder   = new int[start.Count];
            TopSort[] marks      = new TopSort[start.Count];
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
                        Log.Write(LogLevel.Error, "Cyclic dependency detected at {0}!", start[i].GetType().Name);
                        return false;
                    }
                }
            }

            // Now reorder the systems
            for (int i = outOrder.Length-1; i >= 0; i--)
            {
                if (start[outOrder[i]] != null) // Check for an End/Start anchor
                    systemsOrdered.Add(startSystems[outOrder[i]-1]);
            }

            return true;
        }
        static int SortVisit(List<List<int>> dependencies, int index, TopSort[] marks, ref int sorted_curr, int[] out_order)
        {
            if (marks[index] == TopSort.Perm) return 0;
            if (marks[index] == TopSort.Temp) return index;
            marks[index] = TopSort.Temp;
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
            marks    [index] = TopSort.Perm;
            out_order[sorted_curr] = index;
            sorted_curr -= 1;
            return 0;
        }
    }
}
