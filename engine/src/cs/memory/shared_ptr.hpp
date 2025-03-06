// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/memory/ptr_common.hpp"

#include <cstdint>
#include <type_traits>

template<typename Type>
class Shared_Ptr;
template<typename Type>
class Weak_Ptr;

template<typename Type>
class Shared_From_This
{
protected:
    mutable Weak_Ptr<Type> _weak_this;
    
    ~Shared_From_This()
    {
        _weak_this.release();
    }

    Shared_Ptr<Type> shared_from_this()
    {
        return _weak_this.lock();
    }

    template <typename Other_Type>
    friend class Shared_Ptr;
};

template<typename Type>
class Shared_Ptr
{
public:
    template<class... Args>
    static Shared_Ptr<Type> create(Args ... args)
    {
        Shared_Ptr<Type> sp(new Type(args...));
        if constexpr (std::is_base_of<Shared_From_This<Type>, Type>::value) 
        {
            sp->_weak_this = sp;
        } 
        return sp;
    }

    Shared_Ptr()
        : _ptr(nullptr),
        _control_block(nullptr)
    {
    }

    explicit Shared_Ptr(Type* raw_other)
        : _ptr(raw_other),
        _control_block(raw_other ? new Ptr_Control_Block : nullptr)
    {
    }

    explicit Shared_Ptr(Type* raw_other, Ptr_Control_Block* control_block)
        : _ptr(raw_other),
        _control_block(control_block)
    {
        if (_control_block != nullptr)
        {
            _control_block->strong_count += 1;
        }
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

    void release()
    {
        if (!_control_block)
        {
            return;
        }

        // shared_from_this double delete guard
        if (_control_block->strong_count == 1)
        {
#ifdef CS_SHARED_PTR_SHOULD_INVOKE_DESTRUCTOR
            delete _ptr;
#else
            free(_ptr);
#endif //CS_SHARED_PTR_SHOULD_INVOKE_DESTRUCTOR
        }

        _control_block->strong_count -= 1;
        
        if (_control_block->strong_count == 0)
        {
            if (_control_block->weak_count == 0)
            {
                delete _control_block;
            }
        }
    }

    ~Shared_Ptr()
    {
        release();
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

    bool is_valid() const { return _ptr != nullptr && _control_block && _control_block->strong_count > 0; }

    operator bool() const
    {
        return is_valid();
    }

private:
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
    void initialize(const Shared_Ptr<Other_Type>& other)
    {
        if (other._control_block != nullptr)
        {
            other._control_block->strong_count += 1;
            _ptr = static_cast<Type*>(other._ptr);
            _control_block = other._control_block;
        }
    }

protected:
    Type *_ptr;
    Ptr_Control_Block *_control_block;


    template <typename Other_Type>
    friend class Weak_Ptr;

    template <typename Other_Type>
    friend class Shared_Ptr;
};
