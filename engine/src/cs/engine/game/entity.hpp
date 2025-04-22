#pragma once

#include "cs/cs.hpp"
#include "cs/name_id.hpp"
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

#define DERIVED_CLASS_BODY(Derived_Type, Base_Type) \
public: \
    using Base = Base_Type; \
    static constexpr Name_Id static_class_name = Name_Id(#Derived_Type); \
    virtual constexpr Name_Id class_name() const override { return static_class_name; } \
    template<Derived<Base_Class> A> \
    static constexpr bool is_a() \
    { \
        return A::static_class_name == static_class_name || Base::is_a<A>(); \
    }
