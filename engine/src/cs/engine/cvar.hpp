// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/hash_map.hpp"

#include <string>
#include <sstream>
#include <type_traits>

class CVar
{
public:
    Name_Id name;
    std::string description;

    Event<> on_change_event;

public:
    CVar() = default;

    CVar(const Name_Id& name, const std::string& description)
        : name(name), description(description)
    {}

    virtual ~CVar() = default;

    virtual std::string to_string() const = 0;
    virtual void set_from_string(const std::string& value_str) = 0;
};

template <typename Type>
class CVar_T : public CVar
{
public:
    CVar_T() = default;

    CVar_T(const Name_Id& name, const Type& default_value, const std::string& description)
        : CVar(name, description), _default_value(default_value), _value(default_value)
    {
    }

    const Type& get() const { return _value; }

    virtual std::string to_string() const override
    {
        std::ostringstream oss;
        oss << _value;
        return oss.str();
    }

    virtual void set_from_string(const std::string& value_str) override
    {
        std::istringstream iss(value_str);
        Type new_value;
        if (iss >> new_value)
        {
            set(new_value);
        }
    }
    
    void set(const Type& in_value) 
    {
        if (_value != in_value)
        {
            _value = in_value;
            on_change_event.broadcast();
        }
    }

    void reset()
    {
        set(_default_value);
    }

protected:
    Type _default_value;
    Type _value;
};

class CVar_Registry : public Singleton<CVar_Registry>
{
public:
    template <typename Type>
    Shared_Ptr<CVar_T<Type>> register_cvar(const Name_Id& name, const Type& defaultValue, const std::string& description)
    {
        Shared_Ptr<CVar>* p_cvar = _cvars.find(name);
        if (p_cvar != nullptr)
        {
            assert(*p_cvar);
            printf("CVar already exists with description : \"%s\"\n", (*p_cvar)->description.c_str());
            return *p_cvar;
        }
        
        Shared_Ptr<CVar_T<Type>> cvar_t = Shared_Ptr<CVar_T<Type>>::create(name, defaultValue, description);
        _cvars.insert(name, cvar_t);
        return cvar_t;
    }

    void register_cvar(const Shared_Ptr<CVar>& in_cvar)
    {
        if (!in_cvar)
        {
            return;
        }

        Shared_Ptr<CVar>* p_cvar = _cvars.find(in_cvar->name);
        if (p_cvar != nullptr)
        {
            assert(*p_cvar);
            printf("CVar already exists with name \"%s\"\n", (*p_cvar)->name.c_str());
            return;
        }

        _cvars.insert(in_cvar->name, in_cvar);
    }

    Shared_Ptr<CVar> get_cvar(const Name_Id& name)
    {
        Shared_Ptr<CVar>* p_cvar = _cvars.find(name);
        return p_cvar ? *p_cvar : Shared_Ptr<CVar>();
    }
    
    template<typename Type>
    Shared_Ptr<CVar_T<Type>> get_cvar(const Name_Id& name)
    {
        Shared_Ptr<CVar>* p_cvar = _cvars.find(name);
        return p_cvar ? *p_cvar : Shared_Ptr<CVar_T<Type>>();
    }

    void list_all_cvars()
    {
        for (const Pair<Name_Id, Shared_Ptr<CVar>>& pair : _cvars)
        {
            if (!pair.b)
            {
                return;
            }

            printf("\'%s\' : \"%s\"\n", pair.b->name.c_str(), pair.b->description.c_str());
        }
    }

private:
    Hash_Map<Name_Id, Shared_Ptr<CVar>> _cvars;
};
