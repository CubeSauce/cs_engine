// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/containers/linked_list.hpp"

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

            for (Linked_List<Hash_Pair>::Node* it = _pair_linked_lists[i].get_first(); it != nullptr;)
            {
                if (it == nullptr)
                {
                    continue;
                }

                f(it->data);

                it = it->next;
            }
        }
    }

protected:
    Linked_List<Hash_Pair> *_pair_linked_lists;
    int32 _size;
};
