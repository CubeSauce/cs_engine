// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

#include <initializer_list>

template<typename Type>
class Dynamic_Array
{
public:
    Dynamic_Array(int32 initial_capacity = 0)
        :   _data(nullptr),
            _capacity(0),
            _size(0)
    {
        if (initial_capacity > 0)
        {
            increase_capacity(initial_capacity);
        }
    }

    Dynamic_Array(const Dynamic_Array& other)
        :   _data(nullptr),
            _capacity(0),
            _size(0)
    {
        *this = other;
    }

    Dynamic_Array(Dynamic_Array&& other) noexcept
        :   _data(nullptr),
            _capacity(0),
            _size(0)
    {
        *this = other;
    }

    Dynamic_Array(std::initializer_list<Type> in_initializer_list)
        :   _data(nullptr),
            _capacity(0),
            _size(0)
    {
        resize((int32)in_initializer_list.size());
        int32 index = 0;
        for (const Type& value : in_initializer_list)
        {
            _data[index] = value;
            ++index;
        }
    }

    ~Dynamic_Array()
    {
        delete[] _data;
    }

    void add(const Type& other)
    {
        if (_size >= _capacity)
        {
            increase_capacity(_capacity * 2);
        }

        _data[_size] = other;
        _size += 1;
    }
    
    void add(Type &&other)
    {
        if (_size >= _capacity)
        {
            increase_capacity(_capacity * 2);
        }

        _data[_size] = other;
        _size += 1;
    }

    void insert(int32 index, const Type& other)
	{
        if (index < 0)
        {
            return;
        }
        else if (index >= _size)
        {
            add(other);
            return;
        }

		if (_size >= _capacity)
		{
			increase_capacity(_capacity * 2);
		}

        for (int i = _size; i > index; i -= 1)
        {
            _data[i] = _data[i - 1];
        }

        _data[index] = other;
        _size += 1;
    }

    void reserve(int32 new_capacity)
    {
        increase_capacity(new_capacity);
    }

    void resize(int32 new_size)
    {
        increase_capacity(new_size);
        _size = new_size;
    }

    void clear()
    {
        _size = _capacity = 0;   
        delete[] _data;
    }

    int32 size() const { return _size; }
    int32 size_in_bytes() const { return _size * sizeof(Type); }
    int32 capacity() const { return _capacity; }

    Type* begin() { return _data; }
    Type* begin() const { return _data; }
    
    Type* end() { return _data + _size; }
    Type* end() const { return _data + _size; }

    Dynamic_Array<Type>& operator=(const Dynamic_Array<Type>& other)
    {
        increase_capacity(other._capacity);

        for (int32 i = 0; i < other._size; i += 1)
        {
            _data[i] = other._data[i];
        }

        _size = other._size;

        return *this;
    }

    Dynamic_Array<Type>& operator=(Dynamic_Array<Type>&& other) noexcept
    {
        increase_capacity(other._capacity);

        for (int32 i = 0; i < other._size; i += 1)
        {
            _data[i] = CS_MOVE(other._data[i]);
        }

        other._capacity = 0;
        delete[] other._data;
        other._data = nullptr;

        _size = other._size;
        other._size = 0;

        return *this;
    }

    Type& operator[](int32 index) { return _data[index]; }
    const Type& operator[](int32 index) const { return _data[index]; }

private:
    Type* _data;
    int32 _capacity, _size;

private:
    void increase_capacity(int32 new_capacity)
    {
        if (new_capacity == 0)
        {
            new_capacity = 1;
        }

        if (new_capacity <= _capacity)
        {
            return;
        }

        Type* new_data = new Type[new_capacity];
        for (int32 i = 0; i < _size; i += 1)
        {
            new_data[i] = CS_MOVE(_data[i]);
        }

        delete[] _data;
        _data = new_data;
        _capacity = new_capacity;
    }
};
