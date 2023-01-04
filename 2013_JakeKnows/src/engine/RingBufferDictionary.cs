using System;
using System.Linq;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Data.SqlTypes;
using System.Collections.Concurrent;

namespace JakeKnowsEngineComponent
{
    public class RingBufferDictionary<T> : ConcurrentDictionary<SqlInt64, T>
    {
        private ConcurrentDictionary<SqlInt64, DateTime> _lru;
        private Object _lock = new Object();
        private volatile int _count; //why? to avoid using container count which is veeery slooooooowwww......

        private readonly int _Capacity = 0;
        private readonly int _SizeOfRemoval = 0;

        public int GetCapacity()
        {
            return _Capacity;
        }

        public RingBufferDictionary()
        {
            lock (_lock)
            {
                _count = 0;
            }
            _Capacity = 0;
            _SizeOfRemoval = 0;
            _lru = new ConcurrentDictionary<SqlInt64, DateTime>();
        }

        public RingBufferDictionary(int capacity, int sizeOfRemoval)
        {
            lock (_lock)
            {
                _count = 0;
            }
            _Capacity = capacity;
            _SizeOfRemoval = sizeOfRemoval;
            _lru = new ConcurrentDictionary<SqlInt64, DateTime>();
        }

        public bool HasInstance(SqlInt64 id)
        {
            bool res = false;
            if (ContainsKey(id))
            {
                res = true;
            }
            return res;
        }

        public bool CanCheckout(SqlInt64 id)
        {
            return _lru.ContainsKey(id) == false;
        }

        public void Checkin(SqlInt64 id)
        {
            _lru.TryAdd(id, DateTime.Now);
        }

        public T Checkout(SqlInt64 id)
        {
            bool isCheckedOut = CanCheckout(id);
            T instance = default(T);
            if (isCheckedOut == false)
            {
                instance = this.GetOrAdd(id, default(T));
                DateTime t;
                _lru.TryRemove(id, out t);
            }
            return instance;
        }

        public bool Add(SqlInt64 id, T item)
        {
            if (ContainsKey(id) == true)
            {
                return false;
            }
            _lru.TryAdd(id, DateTime.Now);
            TryAdd(id, item);
            bool remove = false;
            lock (_lock)
            {
                _count++;
                if (_Capacity > 0 && _count > _Capacity)
                {
                    remove = true;
                }
            }
            if (remove)
                RemoveElders();
            return true;
        }

        public bool Remove(SqlInt64 id) 
        {
            lock (_lock)
            {
                if (_count == 0)
                    return true;
            }
            if (CanCheckout(id) == false)
                return false;
            DateTime t;
            _lru.TryRemove(id, out t);
            T instance = default(T);
            TryRemove(id, out instance);
            bool clear = false;
            lock (_lock)
            {
                _count--;
                if (_count == 0)
                {
                    clear = true;
                }
            }
            if (clear)
                Clear();
            return true;
        }

        public void RemoveElders()
        {
            int removed = 0;
            bool clear = false;

            var sorted = _lru
                .OrderBy(kv => kv.Key)
                .GroupBy(o => o.Value)
                .Select(g => new
                {
                    Chunks = g.Select((o, i) => new { Val = o, Index = i })
                              .GroupBy(item => item.Index)
                });
            foreach (var item in sorted.SelectMany(item => item.Chunks))
            {
                if (CanCheckout(item.First().Index) == true)
                {
                    DateTime t;
                    _lru.TryRemove(item.First().Index, out t);
                    T instance = default(T);
                    TryRemove(item.First().Index, out instance);
                    removed++;
                    lock (_lock)
                    {
                        _count--;
                    }
                }
                lock (_lock)
                {
                    if (_count == 0)
                    {
                        clear = true;
                    }
                }
                if (clear)
                {
                    Clear();
                    return;
                }
                lock (_lock)
                {
                    if (removed >= _SizeOfRemoval)
                    {
                        return;
                    }
                }
            }
        }

        public new void Clear()
        {
            base.Clear();
            _lru.Clear();
            lock (_lock)
            {
                _count = 0;
            }
        }
    }
}

