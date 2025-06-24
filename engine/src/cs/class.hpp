#pragma once

#include "cs/cs.hpp"
#include "cs/name_id.hpp"
#include "cs/containers/pair.hpp"

#include <functional>

struct Base_Class
{
    static constexpr Name_Id static_class_name = Name_Id("Base_Class");
    virtual constexpr Name_Id class_name() const { return static_class_name; }
    template<Derived<Base_Class> A>
    static constexpr bool is_a()
    {
        return A::static_class_name == static_class_name;
    }
};


// We're supporting 512 classes in the registry
struct Class_Registry
{
    int64 class_register_index{ 0 };
    //TODO: map?
    std::array<Pair<Name_Id, std::function<Base_Class*()>>, 512> id_class_creator_array;
};

Class_Registry& get_class_registry();

#define DERIVED_CLASS_BODY(Derived_Type, Base_Type) \
public: \
    using Base = Base_Type; \
    static constexpr Name_Id static_class_name = Name_Id(#Derived_Type); \
    virtual constexpr Name_Id class_name() const override { return static_class_name; } \
    template<Derived<Base_Class> A> \
    static constexpr bool is_a() \
    { \
        return A::static_class_name == static_class_name || Base::is_a<A>(); \
    } \
    constexpr Base_Class* static_create_instance() \
    { \
        return new Derived_Type(); \
    } \
    struct Register \
    {\
        Register(Class_Registry& class_registry) { \
            class_registry.id_class_creator_array[get_class_registry().class_register_index++] = \
                { static_class_name, []() { return new Derived_Type(); } }; \
        }\
    };\
private:

// If you do not register the class, it won't be available in the reflection system
#define REGISTER_CLASS(Class) Class::Register register_##Class(get_class_registry());

Base_Class* create_class_instance(const Name_Id& class_id)
{
    for (const Pair<Name_Id, std::function<Base_Class*()>>& pair : get_class_registry().id_class_creator_array)
    {
        if (pair.a == class_id)
        {
            assert(pair.b);
            return pair.b();            
        }
    }

    assert(false);
    return nullptr;
}

class Test_Derived : public Base_Class
{
    DERIVED_CLASS_BODY(Test_Derived, Base_Class);
};
