// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

template <typename Type>
class Singleton
{
public:
    Singleton(void)
    {
        assert(!_singleton);
        _singleton = static_cast<Type*>(this);
    }

    ~Singleton(void)
    {
        assert(_singleton);
        _singleton = 0;
    }

    static Type& get(void)
    {
        assert(_singleton);
        return (*_singleton);           
    }

    static Type* get_ptr(void) 
    { 
        return _singleton; 
    }

protected:
    static Type* _singleton;

private:
    Singleton(const Singleton<Type>&);
    Singleton& operator=(const Singleton<Type>&);
};
