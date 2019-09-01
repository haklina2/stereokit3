using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace StereoKit
{
    public class ECSManager
    {
        struct SystemInfo
        {
            public IComponentSystem system;
            public MethodInfo       addMethod;
        };
        static Dictionary<int, SystemInfo> systems = new Dictionary<int, SystemInfo>();

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

        public static void Update()
        {
            foreach(SystemInfo info in systems.Values)
            {
                info.system.Update();
            }
        }

        public static ComponentId Add<T>(ref T item)
        {
            int hash = typeof(T).GetHashCode();
            return new ComponentId { 
                system = hash, 
                index  = (int)systems[hash].addMethod.Invoke(systems[hash].system, new object[]{item})
            };
        }

        static bool IsSubclassOfGeneric(Type generic, Type toCheck)
        {
            while (toCheck != null && toCheck != typeof(object))
            {
                Type curr = toCheck.IsGenericType ? toCheck.GetGenericTypeDefinition() : toCheck;
                if (generic == curr)
                {
                    return true;
                }
                toCheck = toCheck.BaseType;
            }
            return false;
        }
    }
}
