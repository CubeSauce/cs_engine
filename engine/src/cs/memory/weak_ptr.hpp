// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/memory/shared_ptr.hpp"

template<typename Type>
class Weak_Ptr
{
public:
    Weak_Ptr()
        : _ptr(nullptr),
        _control_block(nullptr)
    {
    }

    Weak_Ptr(const Weak_Ptr<Type>& other)
        : _ptr(nullptr),
        _control_block(nullptr)
    {
        initialize(other);
    }

    Weak_Ptr(const Shared_Ptr<Type>& other)
        : _ptr(nullptr),
        _control_block(nullptr)
    {
        initialize(other);
    }

    template <typename Other_Type>
    Weak_Ptr(const Weak_Ptr<Other_Type>& other)
        : _ptr(nullptr),
        _control_block(nullptr)
    {
        initialize(other);
    }

    template <typename Other_Type>
    Weak_Ptr(const Shared_Ptr<Other_Type>& other)
        : _ptr(nullptr),
        _control_block(nullptr)
    {
        initialize(other);
    }

    Weak_Ptr<Type>& operator=(const Weak_Ptr<Type>& other)
    {
        if (_ptr != other._ptr)
        {
            Weak_Ptr<Type> other_shared_ptr(other);
            swap(other_shared_ptr);
        }

        return *this;
    }
    
    template <typename Other_Type>
    Weak_Ptr<Type>& operator=(const Weak_Ptr<Other_Type>& other)
    {
        if (_ptr != other._ptr)
        {
            Weak_Ptr<Type> other_shared_ptr(other);
            swap(other_shared_ptr);
        }

        return *this;
    }

    void release()
    {
        if (!_control_block)
        {
            return;
        }

        _control_block->weak_count -= 1;
        if (_control_block->weak_count == 0)
        {
            delete _control_block;
        }
    }

    ~Weak_Ptr()
    {
        release();
    }

    template<typename Other_Type>
    void reset(Other_Type *other)
    {
        if (_ptr != other)
        {
            Weak_Ptr<Type> temp(other);
            temp.swap(*this);
        }
    }

    template<typename Other_Type>
    void reset(const Weak_Ptr<Other_Type> other)
    {
        if (_ptr != other._ptr)
        {
            Weak_Ptr<Type> temp(other);
            temp.swap(*this);
        }
    }

    Shared_Ptr<Type> lock() const
    {
        if (!is_valid())
        {
            return Shared_Ptr<Type>();
        }

        return Shared_Ptr<Type>(_ptr, _control_block);
    }

    bool operator==(const Weak_Ptr<Type>& other) const
    {
        return _ptr == other._ptr;
    }

    bool operator!=(const Weak_Ptr<Type>& other) const
    {
        return _ptr != other._ptr;
    }

    bool is_valid() const { return _ptr != nullptr && _control_block && _control_block->strong_count > 0; }

    operator bool() const
    {
        return is_valid();
    }

private:
    void swap(Weak_Ptr<Type>& other)
    {
        Type* temp_ptr = _ptr;
        _ptr = other._ptr;
        other._ptr = temp_ptr;

        Ptr_Control_Block* temp_control_block = _control_block;
        _control_block = other._control_block;
        other._control_block = temp_control_block;
    }

    void swap(Shared_Ptr<Type>& other)
    {
        Type* temp_ptr = _ptr;
        _ptr = other._ptr;
        other._ptr = temp_ptr;

        Ptr_Control_Block* temp_control_block = _control_block;
        _control_block = other._control_block;
        other._control_block = temp_control_block;
    }

    template<typename Other_Type>
    void initialize(const Weak_Ptr<Other_Type>& other)
    {
        if (other._control_block != nullptr)
        {
            other._control_block->weak_count += 1;
            _ptr = static_cast<Type*>(other._ptr);
            _control_block = other._control_block;
        }
    }

    template<typename Other_Type>
    void initialize(const Shared_Ptr<Other_Type>& other)
    {
        if (other._control_block != nullptr)
        {
            other._control_block->weak_count += 1;
            _ptr = static_cast<Type*>(other._ptr);
            _control_block = other._control_block;
        }
    }

private:
    Type *_ptr;
    Ptr_Control_Block *_control_block;

    template <typename Other_Type>
    friend class Weak_Ptr;

    template <typename Other_Type>
    friend class Shared_Ptr;
};
