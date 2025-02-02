// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

#include <cstdint>

struct Shared_Ptr_Control_Block
{
    int32 reference_count = 1;
    int32 weak_count = 0;
};

template<typename Type>
class Shared_Ptr
{
public:
    template<class... Args>
    static Shared_Ptr<Type> create(Args ... args)
    {
        return Shared_Ptr<Type>(new Type(args...));
    }

    Shared_Ptr()
        : _ptr(nullptr),
        _control_block(nullptr)
    {
    }

    explicit Shared_Ptr(Type* raw_other)
        : _ptr(raw_other),
        _control_block(raw_other ? new Shared_Ptr_Control_Block : nullptr)
    {
    }

    Shared_Ptr(const Shared_Ptr<Type>& other)
        : _ptr(nullptr),
        _control_block(nullptr)
    {
        initialize(other);
    }

    template <typename Other_Type>
    Shared_Ptr(const Shared_Ptr<Other_Type>& other)
        : _ptr(nullptr),
        _control_block(nullptr)
    {
        initialize(other);
    }

    Shared_Ptr<Type>& operator=(const Shared_Ptr<Type>& other)
    {
        if (_ptr != other._ptr)
        {
            Shared_Ptr<Type> other_shared_ptr(other);
            swap(other_shared_ptr);
        }

        return *this;
    }

    template <typename Other_Type>
    Shared_Ptr<Type>& operator=(const Shared_Ptr<Other_Type>& other)
    {
        if (_ptr != other._ptr)
        {
            Shared_Ptr<Type> other_shared_ptr(other);
            swap(other_shared_ptr);
        }

        return *this;
    }

    ~Shared_Ptr()
    {
        if (!_control_block)
        {
            return;
        }

        _control_block->reference_count -= 1;
        if (_control_block->reference_count == 0)
        {
#ifdef CS_SHARED_PTR_SHOULD_INVOKE_DESTRUCTOR
            delete _ptr;
#else
            free(_ptr);
#endif //CS_SHARED_PTR_SHOULD_INVOKE_DESTRUCTOR

            if (_control_block->weak_count == 0)
            {
                delete _control_block;
            }
        }
    }

    template<typename Other_Type>
    void reset(Other_Type *other)
    {
        if (_ptr != other)
        {
            Shared_Ptr<Type> temp(other);
            temp.swap(*this);
        }
    }

    template<typename Other_Type>
    void reset(const Shared_Ptr<Other_Type> other)
    {
        if (_ptr != other._ptr)
        {
            Shared_Ptr<Type> temp(other);
            temp.swap(*this);
        }
    }

    bool operator==(const Shared_Ptr<Type>& other) const
    {
        return _ptr == other._ptr;
    }

    bool operator!=(const Shared_Ptr<Type>& other) const
    {
        return _ptr != other._ptr;
    }

    Type* get() const
    {
        return _ptr;
    }

    Type* operator->() const
    {
        return _ptr;
    }

    Type& operator*() const
    {
        return *_ptr;
    }

    int32 get_count() const
    {
        return _control_block ? _control_block->reference_count : 1;
    }

    bool is_unique() const
    {
        return get_count() == 1;
    }

    operator bool() const
    {
        return _ptr != nullptr;
    }

private:
    void swap(Shared_Ptr<Type>& other)
    {
        Type* temp_ptr = _ptr;
        _ptr = other._ptr;
        other._ptr = temp_ptr;

        Shared_Ptr_Control_Block* temp_control_block = _control_block;
        _control_block = other._control_block;
        other._control_block = temp_control_block;
    }

    template<typename Other_Type>
    void initialize(const Shared_Ptr<Other_Type>& other)
    {
        if (other._control_block != nullptr)
        {
            other._control_block->reference_count += 1;
            _ptr = static_cast<Type*>(other._ptr);
            _control_block = other._control_block;
        }
    }

private:
    Type *_ptr;
    Shared_Ptr_Control_Block *_control_block;

    template <typename Other_Type>
    friend class Shared_Ptr;
};
