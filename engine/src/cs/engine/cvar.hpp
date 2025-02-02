// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/hash_table.hpp"

#include <string>
#include <sstream>
#include <type_traits>

class CVar
{
public:
    Name_Id name;
    std::string description;

    Event<0> on_change_event;

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
        Shared_Ptr<CVar>& cvar = _cvars[name];
        if (cvar)
        {
            printf("CVar already exists with description : \"%s\"\n", cvar->description.c_str());
            return cvar;
        }
        
        Shared_Ptr<CVar_T<Type>> cvar_t = Shared_Ptr<CVar_T<Type>>::create(name, defaultValue, description);
        cvar = cvar_t;
        return cvar_t;
    }

    void register_cvar(const Shared_Ptr<CVar>& in_cvar)
    {
        if (!in_cvar)
        {
            return;
        }

        Shared_Ptr<CVar>& cvar = _cvars[in_cvar->name];
        if (cvar)
        {
            printf("CVar already exists with name \"%s\"\n", cvar->name.str);
            return;
        }

        cvar = in_cvar;
    }

    Shared_Ptr<CVar> get_cvar(const Name_Id& name)
    {
        return _cvars[name.id];
    }
    
    template<typename Type>
    Shared_Ptr<CVar_T<Type>> get_cvar(const Name_Id& name)
    {
        return _cvars[name.id];
    }

    void list_all_cvars()
    {
        _cvars.for_each([](const Hash_Table<Shared_Ptr<CVar>>::Hash_Pair& pair){
            if (!pair.data)
            {
                return;
            }

            printf("\'%s\' : \"%s\"\n", pair.data->name.str, pair.data->description.c_str());
        });
    }

private:
    Hash_Table<Shared_Ptr<CVar>> _cvars;
};
