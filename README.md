# delegate

Class template `vdk::delegate` is a general-purpose polymorphic function wrapper. Instances of `delegate` can store and invoke any function-like entity with specified signature, such as functions, lambda expressions, bind expressions, or other function objects.

`delegate` satisfies the requirements of MoveConstructible and MoveAssignable, but not the requirements of either CopyConstructible or CopyAssignable. Because of this, stored function object does not have to be copyable; move-only type would suffice.

`delegate` instances can be compared. Two delegates are equal if they both are empty, or if they both contain targets of exactly the same static type with accessible equality comparison operator and the result of their comparison returns `true`. As a consequence, two delegates are not equal if only one of them is empty, or they contain targets of different static types, or the result of comparison of stored targets returns `false`, or the target's type does not provide accessible comparison operator.

To reduce dynamic memory allocation `delegate` implements small buffer optimization technique. When size of a stored callable target is `<= sizeof(ptr_to_any_callable) + sizeof(ptr_to_any_object)` and its move-operations are `noexcept`, small buffer optimization is guaranteed. Such targets are always directly stored inside delegate's internal buffer and no dynamic memory allocation takes place. However, if a target is too large to fit into delegate, or if its move operations may throw, the required memory is allocated dynamically. This implies that pointers to functions are always stored locally within a delegate instance.

A signature given to `delegate` is used to define its function call operator. Any qualifiers and | or specifiers in the signature are applied to the delegate's function call operator and the target is invoked through the specified call path.

Trying to invoke a delegate without a target results in assertion being triggered. If exceptions on failed dynamic memory allocations should be avoided, one could use experimental functionality - custom global memory resource in `vdk::memory::delegate` namespace.

## Usage:

**Note!** `vdk::delegate` requires a compiler that supports **C++17** standard.

To use `delegate` just include `delegate.h` into your project and you are good to go.

GoogleTest is required to build and run tests. CMake files are provided to simplify the process. If you already have a copy of GoogleTest framework, just run CMake and set `GTEST_SOURCE_DIR` cache variable to point to the directory that contains your GoogleTest sources. If you don't have GoogleTest sources, CMake will download and compile them automatically.

`demo` directory contains code examples that can serve as a tutorial for learning how to use `delegate`.

## [API documentation](docs/delegate.md)

## License:

This software is licensed under the MIT License.