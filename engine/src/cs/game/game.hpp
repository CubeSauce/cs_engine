// CS Engine
// Author: matija.martinec@protonmail.com
//
// Contains c++ metaprogramming (template) SOA structures, with some extra "reflection" stuff with Name_Ids
//

#pragma once

#include "cs/cs.hpp"

struct Component
{
	virtual ~Component() = default;
	virtual constexpr Name_Id get_id() const = 0;
};

template<int64 N, typename Type>
struct Component_Bucket
{
	Type component_storage[N];
};

template<typename Type>
struct Component_Handle
{
	int64 index{-1};

	bool is_valid() const { return index != -1; }
};

template<int64 N, Derived<Component>...ComponentTypes>
struct Component_Storage : private Component_Bucket<N, ComponentTypes>...
{
	template<typename ComponentType>
	ComponentType& get(int64 Index)
	{
		return Component_Bucket<N, ComponentType>::component_storage[Index];
	}

	template<typename ComponentType>
	ComponentType* get()
	{
		return Component_Bucket<N, ComponentType>::component_storage;
	}
};

template<typename Type>
struct Dynamic_Component_Bucket
{
	Dynamic_Array<Type> component_storage;
};

template<Derived<Component>...ComponentTypes>
struct Dynamic_Component_Storage : private Dynamic_Component_Bucket<ComponentTypes>...
{
	Dynamic_Component_Storage()
	{
	}

	void initialize(int64 N = 256)
	{
		printf("Initializing Dynamic Component Storage with components: \n");
		([&] {
			Dynamic_Component_Bucket<ComponentTypes>::component_storage.reserve(N);
			Dynamic_Component_Bucket<ComponentTypes>::component_storage.clear();
			printf("  \'%s\'\n", ComponentTypes::id.c_str());
		}(), ...);
	}

	template<typename ComponentType>
	Component_Handle<ComponentType> add()
	{
		Component_Handle<ComponentType> handle;
		auto& array = get_array<ComponentType>();
		handle.index = array.size();
		array.push_back(ComponentType{});
		return handle;
	}

	template<typename ComponentType>
	ComponentType& add(Component_Handle<ComponentType>& out_handle)
	{
		auto& array = get_array<ComponentType>();
		out_handle.index = array.size();
		array.push_back(ComponentType{});
		return array[out_handle.index];
	}

	template<typename ComponentType>
	ComponentType& get(const Component_Handle<ComponentType>& index)
	{
		return Dynamic_Component_Bucket<ComponentType>::component_storage[index.index];
	}

	template<typename ComponentType>
	ComponentType& get(int64 index)
	{
		return Dynamic_Component_Bucket<ComponentType>::component_storage[index];
	}

	template<typename ComponentType>
	Dynamic_Array<ComponentType>& get_array()
	{
		return Dynamic_Component_Bucket<ComponentType>::component_storage;
	}
};
