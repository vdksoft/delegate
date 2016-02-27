C++ Delegate
============

## Introduction

A delegate is a class that represents references to functions with a particular parameter list and return type. When a delegate is instantiated, it can be associated with any function with a compatible signature and return type. This function can be invoked later through the delegate instance. In short, a delegate is a form of type-safe function pointer that specifies a function to call and optionally an object to call this function on.

## Overview

**vdk::delegate** is an implementation of delegate concept for C++. It is a general-purpose type-safe function pointer wrapper. Instances of **vdk::delegate** can store, copy, and invoke pointers to any callable target such as free (global) function, static member function of some class (static method), member function of some class (method), function object (functor), or lambda. Any of those that matches the delegate type – has a compatible signature and return type, can be assigned to the delegate and invoked later.

To assign some function / method / functor to **vdk::delegate** instance user passes function pointer / object pointer and method pointer / functor pointer to the delegate’s constructor. To invoke the delegate *"operator()"* is used. If target function assigned to the delegate has an argument list, all arguments must be passed into the *operator()*.

Any delegate can be in one of two states: either it is associated with some target function or it is empty. In latter case it is equal to null pointer. Attempting to invoke a null delegate results in undefined behavior, just like attempting to dereference regular null pointer.

**vdk::delegate** satisfies the requirements of Copy-constructible and Copy-assignable, so it is possible to store instances of this class in Standard Library containers. It is also possible to compare delegates for equality. Two delegates are considered to be equal if they point to the same function / object and method / functor. **vdk::delegate** is quite fast and does not use any dynamic memory allocation at all. On all main platforms it’s size is either equal or less than the size of std::function.

In order to use this library, compiler must be compatible with at least C++11 Standard. **vdk::delegate** relies on Standard C++ only, and does not use any hacks, non-standard extensions or third party libraries.

## Design Goals

1. Delegate must be as simple to use as possible, and must have clean and intuitive syntax;
2. Delegate must behave as generalized function pointer, without any unnecessary functionality;
3. Creation, copying, assigning, and invoking delegate instances must be as fast as possible;
4. Delegate must not use any dynamic memory allocation;
5. Delegate must rely on Standard C++ only without any non-standard extensions, hacks, or third party libraries;
6. Delegate instances must be comparable to identify whether they point to the same function;
7. Delegate must be copy-constructible and copy-assignable to be used in Standard containers;
8. Delegate must be able to work with methods with qualifiers: *const*, *volatile*, *const volatile*.

## Performance

Performance of **vdk::delegate** was measured on Windows (Visual Studio 2015 CE) and Linux (GCC 4.8.5) platforms in so called "release" builds (with optimizations). Tests showed that performance of **vdk::delegate** is almost identical to two pointer dereference operations. Such overhead is not that much even for performance critical applications, so it should be acceptable in majority of cases.

### **Documentation, tutorial and API reference can be found in the package.**