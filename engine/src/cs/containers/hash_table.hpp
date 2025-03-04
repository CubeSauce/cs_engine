// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/containers/linked_list.hpp"

#include <unordered_map>

#include <functional>

template <typename Type>
class Hash_Table
{
public:
    struct Hash_Pair
    {
        uint32 hash;
        Type data;

        Hash_Pair(uint32 hash, const Type& other = Type())
            :   hash(hash),
                data(other)
        {
        }

        bool operator==(const Hash_Pair& other)
        {
            return hash == other.hash;
        }
    };

public:
    Hash_Table(int32 _size = 2048)
        :_size(_size)
    {
        _pair_linked_lists = new Linked_List<Hash_Pair>[_size];
    }

    ~Hash_Table()
    {
        delete[] _pair_linked_lists;
    }

    Type* find(uint32 hash)
    {
        uint32 index = hash % _size;
        Hash_Pair hashToFind(hash);
        _pair_linked_lists[index].find(hashToFind);
    }

    Type& operator[](uint32 hash)
    {
        uint32 index = hash % _size;
        Hash_Pair hashToFind(hash);
        Hash_Pair& a = _pair_linked_lists[index].find_or_add(hashToFind);

        return a.data;
    }

    void for_each(std::function<void(const Hash_Pair& pair)> f)
    {
        if (!_pair_linked_lists)
        {
            return;
        }

        for (int32 i = 0; i < _size; ++i)
        {
            if (_pair_linked_lists[i].get_size() == 0)
            {
                continue;
            }

            for (Linked_List_Node<Hash_Pair>* it = _pair_linked_lists[i].get_first(); it != nullptr;)
            {
                f(it->data);
                it = it->next;
            }
        }
    }

protected:
    Linked_List<Hash_Pair> *_pair_linked_lists;
    int32 _size;
};

template <typename Type>
class Hash_Map
{
public:
    struct Entry
    {
        Type value;
        uint32 hash { 0 };
        int32 next_collision_index { -1 };
    };

public:
    Hash_Map(int32 size = 256)
        : _size(size)
    {
        _entries = new Entry[size];
        _collision_entries = new Entry[size];
    }

    ~Hash_Map()
    {
        delete[] _entries;
    }

    int32 get_index_from_hash(uint32 hash) const
    {
        return hash % _size;
    }

    void add(uint32 hash, const Type& data)
    {
        const int32 key_index = get_index_from_hash(hash);
        if (_entries[key_index].hash == hash)
        {
            // Just change the value
            _entries[key_index].value = data;
            return;
        }

        if (_entries[key_index].hash > 0) // Collision
        {
            int32 next_collision_index = key_index;
            int32 last_collision_index = -1;

            while (_collision_entries[next_collision_index].hash > 0 && _collision_entries[next_collision_index].hash != hash && next_collision_index < _size)
            {
                last_collision_index = next_collision_index;
                if (_collision_entries[next_collision_index].next_collision_index != -1)
                {
                    next_collision_index = _collision_entries[next_collision_index].next_collision_index;
                }
                else
                {
                    next_collision_index++;
                }
            }

            // If true, we reached the maximum amount of items in table (maybe do resizing?)
            assert(next_collision_index < _size);
            
            if (_collision_entries[next_collision_index].hash == hash)
            {
                _collision_entries[next_collision_index].value = data;
                return;
            }

            if (_entries[key_index].next_collision_index == -1)
            {
                _entries[key_index].next_collision_index = next_collision_index;
            }
            else
            {
                _collision_entries[last_collision_index].next_collision_index = next_collision_index;
            }

            _collision_entries[next_collision_index].value = data;
            _collision_entries[next_collision_index].hash = hash;
            
            return;
        }
        
        _entries[key_index].value = data;
        _entries[key_index].hash = hash;
    }

    Type* find(uint32 hash)
    {
        const int32 key_index = get_index_from_hash(hash);

        if (_entries[key_index].hash == hash)
        {
            return &_entries[key_index].value;
        }

        int32 next_collision_index = key_index;
        while (_collision_entries[next_collision_index].hash > 0 && _collision_entries[next_collision_index].hash != hash && next_collision_index < _size)
        {
            next_collision_index = _collision_entries[next_collision_index].next_collision_index;
        }

        if (_collision_entries[next_collision_index].hash == hash)
        {
            return &_collision_entries[next_collision_index].value;
        }

        return nullptr;
    }

    Type* find(uint32 hash) const
    {
        const int32 key_index = get_index_from_hash(hash);

        if (_entries[key_index].hash == hash)
        {
            return &_entries[key_index].value;
        }

        int32 next_collision_index = key_index;
        while (_collision_entries[next_collision_index].hash > 0 && _collision_entries[next_collision_index].hash != hash && next_collision_index < _size)
        {
            next_collision_index = _collision_entries[next_collision_index].next_collision_index;
        }

        if (_collision_entries[next_collision_index].hash == hash)
        {
            return &_collision_entries[next_collision_index].value;
        }

        return nullptr;
    }

    Type& find_or_add(uint32 hash)
    {
        const int32 key_index = get_index_from_hash(hash);
        if (_entries[key_index].hash == hash)
        {
            // Just change the value
            return _entries[key_index].value;
        }

        if (_entries[key_index].hash > 0) // Collision
        {
            int32 next_collision_index = key_index;
            int32 last_collision_index = -1;

            while (_collision_entries[next_collision_index].hash > 0 && _collision_entries[next_collision_index].hash != hash && next_collision_index < _size)
            {
                last_collision_index = next_collision_index;
                if (_collision_entries[next_collision_index].next_collision_index != -1)
                {
                    next_collision_index = _collision_entries[next_collision_index].next_collision_index;
                }
                else
                {
                    next_collision_index++;
                }
            }

            // If true, we reached the maximum amount of items in table (maybe do resizing?)
            assert(next_collision_index < _size);
            
            if (_collision_entries[next_collision_index].hash == hash)
            {
                return _collision_entries[next_collision_index].value;
            }

            _collision_entries[next_collision_index].hash = hash;
            return _collision_entries[next_collision_index].value;
        }

        _entries[key_index].hash = hash;
        return _entries[key_index].value;
    }

private:
    int32 _size { 0 };
    Entry* _entries { nullptr };
    Entry* _collision_entries { nullptr };
};
