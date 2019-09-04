using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace StereoKit
{
    static class ECSThreads
    {
        static List<Action> _queue = new List<Action>();
        static Thread[]     _threads = null;
        static bool         _run = true;
        static int          _active = 0;

        public static bool Busy { get{ return _active > 0 || _queue.Count > 0; } }
        public static int ThreadCount { get{ return Environment.ProcessorCount/2; } }

        public static void Enqueue(Action task)
        {
            if (_threads == null)
                Start();

            lock(_queue) { 
                _queue.Add(task);
            }
        }

        static void Start()
        {
            _threads = new Thread[ThreadCount];
            for (int i = 0; i < _threads.Length; i++) {
                _threads[i] = new Thread(Process);
                _threads[i].Name = "ECS #"+i;
                _threads[i].Start();
            }
        }

        static void Process()
        {
            Action task = null;
            while (_run)
            {
                lock (_queue)
                {
                    if (_queue.Count > 0)
                    {
                        _active += 1;
                        task = _queue[_queue.Count-1];
                        _queue.RemoveAt(_queue.Count-1);
                    }
                }
                if (task != null) { 
                    task();
                    _active -= 1;
                    task = null;
                }
            }
        }

        public static void Wait()
        {
            if (!Busy)
                return;

            Action task = null;
            while (Busy)
            {
                lock (_queue)
                {
                    if (_queue.Count > 0)
                    {
                        _active += 1;
                        task = _queue[_queue.Count - 1];
                        _queue.RemoveAt(_queue.Count - 1);
                    }
                }
                if (task != null)
                {
                    task();
                    _active -= 1;
                    task = null;
                }
            }
        }
    }
}
