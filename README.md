# tag_ptr

## About

This is a simple C++11 implementation of a [tagged pointer](https://en.wikipedia.org/wiki/Tagged_pointer), that allows to use the least significant bits in a pointer to save a payload, generally called `tag`. It does so by exploiting the alignment of types in memory.

In the x86 architecture the data is generally aligned to a multiple of its size, if you do not request your compiler to do otherwise. This means that, for example, a 32 bit integer will have his memory address a multiple of four, thus leaving the two least significant bits to us as storage. Care must be obviously taken when pointer arithmetic or custom alignment is involved as things may break.

The rationale behind this implementation is to closely follow the normal pointer semantic, trying to behave as much as possible as a raw pointer. This thus follows the same ideas behind `unique_ptr` and `shared_ptr` provided by the `stl`.

Because of this there are a few things to note about the design:
* No move constructor or move assignment operator is explicitly provided (and should not be provided implicitly by the compiler either) since move operations on primitive types are simple copies
* The single parameter constructor is marked `explicit` to prevent errors from coercions, following `unique_ptr` semantics as stated
* All the functions and operator overloads behave as on raw pointer ignoring the tag, including the comparison operators
* An exception to this is the `reset` member which overwrites the value held by the `tag_ptr` with the provided one, thus losing the tag inside
* Because of the previous points, the `swap` and `operator<<` overloads are provided, but an `hash` overload is not since pointers with the same address but different tags would be treated as equal keys (this may or may not be the intended behaviour and an eventual implementation is left to the user)
* A `make_tag` function is provided for no reason other than uniformity

## Requirements

The only requirements is a fairly recent C++11 compiler with variadic templates and constexpr support. It is written to be as portable as possible, and it compiles under `GCC` and `clang` using the `-Wall` and `-Werror` flags, and under `MSCV14` using the `/Wall` and `/WX` flags.

If you encounter any error/problem feel free to write me about it via email or by opening an issue here on GitHub.

## Example

```cpp
#include "tag_ptr.hpp"

constexpr uint8_t NO_ERRORS = 0;
constexpr uint8_t SOMETHING_WRONG = 1;
constexpr uint8_t ANOTHER_FLAG = 2;

int main(int argc, char* argv[])
{
	// check how much space we have, uint64_t is aligned to 8 bytes so 3 bits available
	std::cout << (uint32_t)tag_ptr<uint64_t>::tag_bits << std::endl;

	uint64_t value = 12345;
	auto pointer = make_tag<uint64_t>(&value, NO_ERRORS);

	std::cout << (uint32_t)pointer.tag() << std::endl; // prints 0
	pointer.tag(SOMETHING_WRONG);
	std::cout << (uint32_t)pointer.tag() << std::endl; // now prints 1

	auto pointer2 = pointer;
	pointer2.tag(ANOTHER_FLAG);
	assert(pointer == pointer2); // this won't trigger even if the tags are different
	assert(*pointer == *pointer2); // the pointed value is the same too

	pointer.reset(); // now pointer is equal to a nullptr and the tag is 0
	if (pointer)
	{
		// We wil never reach this block
		std::cout << "Pointer is valid!" << std::endl;
	}
}
```