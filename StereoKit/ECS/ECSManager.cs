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
        
        static Dictionary<int, SystemInfo> systems  = new Dictionary<int, SystemInfo>();
        
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
                    Log.Write(LogLevel.Info, "Added component system for <~CYN>{0}<~clr>", type.Name);
                }
            }
        }

        public static void Shutdown()
        {
            foreach (SystemInfo info in systems.Values)
            {
                info.system.Shutdown();
            }
        }

        public static void Update()
        {
            Stopwatch stopWatch = new Stopwatch();
            stopWatch.Start();

            foreach (SystemInfo info in systems.Values)
            {
                info.system.Update();
            }

            stopWatch.Stop();
            Console.WriteLine("Update " + stopWatch.Elapsed.TotalMilliseconds + "ms");
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
    }
}
