/*
  Copyright (c) 2016, Matteo Bertello
  All rights reserved.

  Email:   bertello.matteo@gmail.com
  GitHub:  http://github.com/Corralx/tag_ptr
  Website: http://corralx.me
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

#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace tag_impl {
    constexpr std::size_t log2(std::size_t n) { return n == 1 ? 0 : log2(n / 2) + 1; }
}

template<typename T>
class tag_ptr {
    using pointer = T *;
    using element_type = T;

public:
    tag_ptr() : _ptr(nullptr) {}

    explicit tag_ptr(pointer ptr, std::uint8_t value = 0) : _ptr(ptr) { tag(value); }

    tag_ptr(const tag_ptr &o) : _ptr(o._ptr) {}

    ~tag_ptr() = default;

    tag_ptr &operator=(const tag_ptr &o) {
        _ptr = o._ptr;
        return *this;
    }

#pragma warning (disable: 4800)

    explicit operator bool() const {
        return static_cast<bool>(_ptr_bits & ~static_cast<std::uintptr_t>(tag_mask));
    }

#pragma warning (default: 4800)

    T *get() const {
        return reinterpret_cast<pointer>(_ptr_bits & ptr_mask);
    }

    void reset(pointer p = nullptr) {
        _ptr = p;
    }

    std::uint8_t tag() const {
        return static_cast<std::uint8_t>(_ptr_bits & static_cast<std::uintptr_t>(tag_mask));
    }

    void tag(std::uint8_t value) {
        if ((value & ptr_mask) != 0) {
            throw std::runtime_error("tag is too big");
        }
        _ptr_bits = reinterpret_cast<std::uintptr_t>(get()) | static_cast<std::uintptr_t>(value & tag_mask);
    }

    void swap(tag_ptr &o) {
        pointer tmp = _ptr;
        o._ptr = _ptr;
        _ptr = tmp;
    }

    element_type &operator*() const { return *get(); }

    pointer operator->() const { return get(); }

    static constexpr std::size_t tag_bits = tag_impl::log2(alignof(element_type));

    // mask where the lowest `tag_bits` are set
    // example: for 8 byte alignment tagMask = alignedTo - 1 = 8 - 1 = 7 = 0b111
    static constexpr std::uint8_t tag_mask = alignof(element_type) - static_cast<std::uint8_t>(1);

    // ptr_mask it the contraty: all bits apart from the lowest `tag_bits` are set
    static constexpr std::uint8_t ptr_mask = ~tag_mask;

private:
    union {
        pointer _ptr;
        std::uintptr_t _ptr_bits;
    };
};

template<typename T, typename... Args>
tag_ptr<T> make_tag(Args &&... args) {
    return tag_ptr<T>(std::forward<Args>(args)...);
}

template<typename T>
std::ostream &operator<<(std::ostream &os, tag_ptr<T> ptr) {
    return os << ptr.get();
}

template<typename T>
void swap(tag_ptr<T> &p1, tag_ptr<T> &p2) {
    p1.swap(p2);
}

template<typename T, typename U>
bool operator==(const tag_ptr<T> lhs, const tag_ptr<U> rhs) {
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
bool operator!=(const tag_ptr<T> lhs, const tag_ptr<U> rhs) {
    return lhs.get() != rhs.get();
}

template<typename T, typename U>
bool operator<(const tag_ptr<T> lhs, const tag_ptr<U> rhs) {
    return lhs.get() < rhs.get();
}

template<typename T, typename U>
bool operator>(const tag_ptr<T> lhs, const tag_ptr<U> rhs) {
    return lhs.get() > rhs.get();
}

template<typename T, typename U>
bool operator<=(const tag_ptr<T> lhs, const tag_ptr<U> rhs) {
    return lhs.get() <= rhs.get();
}

template<typename T, typename U>
bool operator>=(const tag_ptr<T> lhs, const tag_ptr<U> rhs) {
    return lhs.get() >= rhs.get();
}
