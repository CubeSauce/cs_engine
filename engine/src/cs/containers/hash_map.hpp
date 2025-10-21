#pragma once

#include "cs/cs.hpp"
#include "cs/containers/pair.hpp"

#include <functional>

template<typename Key, typename Value>
struct Hash_Map_Entry
{
    Pair<Key, Value> pair;
    bool occupied { false };
    bool deleted { false };

    Hash_Map_Entry() = default;

    void set(const Key& key, const Value& value)
    {
        pair.a = key;
        pair.b = value;
        occupied = true;
        deleted = false;
    }

    void clear()
    {
        occupied = false;
        deleted = true;
    }

    bool is_valid() { return occupied && !deleted; }
};

template<typename Key, typename Value>
class Hash_Map_Iterator
{
public:
    using Entry = Hash_Map_Entry<Key, Value>;
    using Type = Pair<Key, Value>;

    Hash_Map_Iterator(Entry* entries, int64 index, int64 capacity)
        :_entries(entries), _index(index), _capacity(capacity)
    {
        _skip_invalid_entry();
    }

    Type& operator*() { return _entries[_index].pair; }
    Type* operator->() { return &_entries[_index].pair; }

    Hash_Map_Iterator<Key, Value>& operator++()
    {
        ++_index;
        _skip_invalid_entry();
        return *this;
    }

    bool operator==(const Hash_Map_Iterator& other) const
    {
        return _index == other._index;
    }

    bool operator!=(const Hash_Map_Iterator& other) const
    {
        return !(*this == other);
    }

private:
    Entry* _entries;
    int64 _index, _capacity;

    void _skip_invalid_entry()
    {
        while (_index < _capacity && !_entries[_index].is_valid())
        {
            ++_index;
        }
    }
};

template<typename Key, typename Value, typename Hash_Function = std::hash<Key>>
class Hash_Map
{
public:
    using Entry = Hash_Map_Entry<Key, Value>;
    using Iterator = Hash_Map_Iterator<Key, Value>;

    Hash_Map(int64 initial_capacity = 32)
        : _capacity(initial_capacity), _size(0)
    {
        _entries = new Entry[_capacity];
    }

    ~Hash_Map()
    { 
        delete[] _entries;
    }

    Hash_Map(const Hash_Map& other)
    {
        _resize(other._capacity);
        _size = 0;

        for (int64 i = 0; i < _capacity; ++i)
        {
            if (other._entries[i].is_valid())
            {
                insert(other._entries[i].pair.a, other._entries[i].pair.b);
            }
        }
    }

    Hash_Map& operator=(const Hash_Map& other)
    {
        _resize(other._capacity);
        _size = 0;

        for (int64 i = 0; i < _capacity; ++i)
        {
            if (other._entries[i].is_valid())
            {
                insert(other._entries[i].pair.a, other._entries[i].pair.b);
            }
        }

        return *this;
    }

    void reserve(int64 capacity)
    {
        _resize(capacity);
    }

    void clear()
    {
        for (int32 i = 0; i < _capacity; ++i)
        {
            if (_entries[i].is_valid())
            {
                _entries[i].clear();
            }
        }
        _size = 0;
    }

    Value* insert(const Key& key, const Value& value)
    {
        if ((_size + 1.0) / _capacity > _max_load_factor) 
        {
            _resize(_capacity * 2);
        }

        int64 idx = _probe_insert(key);
        // replace the value
        if (_entries[idx].occupied && !_entries[idx].deleted)
        {
            _entries[idx].pair.b = value;
            return nullptr;
        }

        _entries[idx].set(key, value);
        ++_size;

        return &_entries[idx].pair.b;
    }

    Value* find(const Key& key)
    {
        int64 idx = _probe_search(key);
        if (idx >= 0 && _entries[idx].is_valid())
        {
            return &_entries[idx].pair.b;
        }
        return nullptr;
    }

    const Value* find(const Key& key) const
    {
        int64 idx = _probe_search(key);
        if (idx >= 0 && _entries[idx].is_valid())
        {
            return &_entries[idx].pair.b;
        }
        return nullptr;
    }

    Value& find_or_add(const Key& key)
    {
        Value* found = find(key);
        if (found)
        {
            return *found;
        }

        return *insert(key, Value{});
    }

    const Value& find_or_add(const Key& key) const
    {
        Value* found = find(key);
        if (found)
        {
            return *found;
        }

        return *insert(key, Value{});
    }

    bool erase(const Key& key)
    {
        int64 idx = _probe_search(key);
        if (idx >= 0 && _entries[idx].is_valid())
        {
            _entries[idx].clear();
            --_size;
            return true;
        }
        return false;
    }

    int64 get_size() const { return _size; }
    int64 get_capacity() const { return _capacity; }

    Iterator begin() { return Iterator(_entries, 0, _capacity); }
    Iterator begin() const { return Iterator(_entries, 0, _capacity); }
    Iterator end() { return Iterator(_entries, _capacity, _capacity); }
    Iterator end() const { return Iterator(_entries, _capacity, _capacity); }

private:
    Entry* _entries;
    int64 _capacity, _size;
    float _max_load_factor = 0.7f;
    Hash_Function _hash_function;

    int64 _probe_insert(const Key& key)
    {
        int64 idx = _hash_function(key) % _capacity;
        int64 start = idx;

        while (_entries[idx].occupied && !_entries[idx].deleted && _entries[idx].pair.a != key)
        {
            idx = (idx + 1) % _capacity;
            assert(idx != start);
        }

        return idx;
    }

    int64 _probe_search(const Key& key) const
    {
        int64 idx = _hash_function(key) % _capacity;
        int64 start = idx;

        while (_entries[idx].occupied)
        {
            if (!_entries[idx].deleted && _entries[idx].pair.a == key)
            {
                return idx;
            }

            idx = (idx + 1) % _capacity;
            if (idx == start) break;
        }

        return -1; // sentinel value for not found
    }

    void _resize(int64 new_cap) 
    {
        Entry* old_entries = _entries;
        int64 old_capacity = _capacity;

        _entries = new Entry[new_cap];
        _capacity = new_cap;
        _size = 0;

        for (int64 i = 0; i < old_capacity; ++i)
        {
            if (old_entries[i].is_valid())
            {
                insert(old_entries[i].pair.a, old_entries[i].pair.b);
            }
        }

        delete[] old_entries;
    }
};
