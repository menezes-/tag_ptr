/*
  Copyright (c) 2016, Matteo Bertello
  All rights reserved.

  Website: http://corralx.me
  Email:   bertello.matteo@gmail.com
  Twitter: http://twitter.com/corralx

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * The names of its contributors may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.
*/

#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>

// NOTE(Corralx): This prevents the clash of name with log2 from the stl
namespace impl
{

template<size_t V>
struct log2
{
	enum { value = log2<V / 2>::value + 1 };
};

template<>
struct log2<1>
{
	enum { value = 0 };
};

}

// NOTE(Corralx): No move constructor/move assignment operator, move semantic for primitive types is copy
// NOTE(Corralx): No std::hash specialization because of the semantics of comparison operators
template<typename T>
class tag_ptr
{
public:
	tag_ptr() : _ptr(nullptr) {}
	// NOTE(Corralx): This is marked explicit to prevent copy assignment from coercion
	explicit tag_ptr(T* ptr, uint8_t value = 0) : _ptr(ptr) { data(value); }
	tag_ptr(const tag_ptr& o) : _ptr(o._ptr) {}
	~tag_ptr() = default;

	tag_ptr& operator=(const tag_ptr& o)
	{
		_ptr = o._ptr;
		return *this;
	}

// NOTE(Corralx): This prevents Visual Studio from complaining about the pointer to bool cast with /W3
#pragma warning (disable: 4800)
	operator bool() const
	{
		return static_cast<bool>(_ptr_bits & ~static_cast<uintptr_t>(data_mask));
	}
#pragma warning (default: 4800)

	T* get() const
	{
		return reinterpret_cast<T*>(_ptr_bits & ~data_mask);
	}

	// NOTE(Corralx): This resets both the pointer and the data
	void reset(T* p = nullptr)
	{
		_ptr = p;
	}

	uint8_t data() const
	{
		return static_cast<uint8_t>(_ptr_bits & static_cast<uintptr_t>(data_mask));
	}

	void data(uint8_t value)
	{
		// NOTE(Corralx): check that we have enough space to actually store the data
		assert((value & data_mask) == value);
		_ptr_bits = reinterpret_cast<uintptr_t>(get()) |
			static_cast<uintptr_t>(value & data_mask);
	};

	void swap(tag_ptr& o)
	{
		T* tmp = _ptr;
		o._ptr = _ptr;
		_ptr = tmp;
	}

	T& operator*() const { return *get(); }
	T* operator->() const { return get(); }

	static constexpr uint8_t data_bits = impl::log2<alignof(T)>::value;
	static constexpr uint8_t data_mask = alignof(T) - (uint8_t)1;

private:
	union
	{
		T* _ptr;
		uintptr_t _ptr_bits;
	};
};

template<typename T, typename... Args>
tag_ptr<T> make_tag(Args&&... args)
{
	return tag_ptr<T>(std::forward<Args>(args)...);
}

template<typename T>
std::ostream &operator<<(std::ostream& os, tag_ptr<T> ptr)
{
	return os << ptr.get();
}

template<typename T>
void swap(tag_ptr<T>& p1, tag_ptr<T>& p2)
{
	p1.swap(p2);
}

template<typename T, typename U>
bool operator==(const tag_ptr<T> lhs, const tag_ptr<U> rhs)
{
	return lhs.get() == rhs.get();
}

template<typename T, typename U>
bool operator!=(const tag_ptr<T> lhs, const tag_ptr<U> rhs)
{
	return lhs.get() != rhs.get();
}

template<typename T, typename U>
bool operator<(const tag_ptr<T> lhs, const tag_ptr<U> rhs)
{
	return lhs.get() < rhs.get();
}

template<typename T, typename U>
bool operator>(const tag_ptr<T> lhs, const tag_ptr<U> rhs)
{
	return lhs.get() > rhs.get();
}

template<typename T, typename U>
bool operator<=(const tag_ptr<T> lhs, const tag_ptr<U> rhs)
{
	return lhs.get() <= rhs.get();
}

template<typename T, typename U>
bool operator>=(const tag_ptr<T> lhs, const tag_ptr<U> rhs)
{
	return lhs.get() >= rhs.get();
}
