# `class delegate<Fn>`

## Methods:

-----------------------

1. **`delegate() noexcept = default;`**

2. **`delegate(std::nullptr_t) noexcept;`**

3. **`delegate(fn_t func) noexcept;`**

4. **`template<typename F>`**  
**`delegate(F func) noexcept(is_internal_v<F>);`**

5. **`delegate(delegate && other) noexcept;`**

Constructs a `delegate` instance.

1-2: Creates a null (empty) delegate.

3: Creates a delegate with the pointer to function `func`. This constructor does not participate in overload resolution unless `func` is a pointer to function with exact signature as the delegate's signature.

4: Creates a delegate with the function object `func`. This constructor does not participate in overload resolution unless `func` is callable for argument types and return type specified in the delegate's signature.

5: Moves the target of `other` to the target of `*this`. If `other` is empty, `*this` will be empty after the call too. `other` is in a valid but unspecified state after the call.

If `func` is a null pointer, `*this` will be empty after the call. For (4), if `func`'s size or alignment does not fit into internal delegate's storage, or if `func`'s move operations may throw an exception, the required memory is allocated dynamically.

-----------------------

1. **`delegate & operator=(delegate && other) noexcept;`**

2. **`delegate & operator=(std::nullptr_t) noexcept;`**

3. **`delegate & operator=(fn_t func) noexcept;`**

4. **`template<typename F>`**  
**`delegate & operator=(F func) noexcept(is_internal_v<F>);`**

Assigns a new target to the `delegate`.

1: Moves the target of `other` to the target of `*this`. If `other` is empty, `*this` will be empty after the call as well. `other` is in a valid but unspecified state after the call.

2: Drops the current target. `*this` is empty after the call.

3: Sets the target of `*this` to the pointer to function `func`. This operator does not participate in overload resolution unless `func` is a pointer to function with exact signature as the delegate's signature.

4: Sets the target of `*this` to the function object `func`. This operator does not participate in overload resolution unless `func` is callable for argument types and return type specified in the delegate's signature.

If `func` is a null pointer, `*this` will be unchanged after the call. For (4), if `func`'s size or alignment does not fit into internal delegate's storage, or if `func`'s move operations may throw an exception, the required memory is allocated dynamically.

-----------------------

**`delegate(const delegate &) = delete;`**

**`delegate & operator=(const delegate &) = delete;`**

The copy constructor and copy assignment operators are deleted. `delegate` is move-only.

-----------------------

**`~delegate() noexcept;`**

Destroys the `delegate` instance. If the delegate is not empty, its target is destroyed as well.

-----------------------

**`operator()(ArgTs ... args); |qualifiers-specifiers depend on the delegate's signature|`**

Invokes the stored callable target with the provided arguments `args`.
If `*this` is empty - assertion is triggered.
Returns the result of invoking the stored target.

-----------------------

**`explicit operator bool() const noexcept;`**

Checks whether `*this` stores a callable target, i.e. is not empty.
Returns `true` if `*this` is not empty, `false` otherwise.

-----------------------

**`bool operator==(const delegate & other) const noexcept;`**

**`bool operator!=(const delegate & other) const noexcept;`**

Compares `delegate` instances. If their targets are of exactly the same static type and equality comparable, returns the result of comparison; otherwise returns `false`.

-----------------------

## Functions:

**`template<typename Fn>`**  
**`bool operator==(const delegate<Fn> & lhs, std::nullptr_t) noexcept;`**

**`template<typename Fn>`**  
**`bool operator==(std::nullptr_t, const delegate<Fn> & rhs) noexcept;`**

**`template<typename Fn>`**  
**`bool operator!=(const delegate<Fn> & lhs, std::nullptr_t) noexcept;`**

**`template<typename Fn>`**  
**`bool operator!=(std::nullptr_t, const delegate<Fn> & rhs) noexcept;`**

Compares a `delegate` instance with a null pointer.
Returns `true` if the delegate is empty, `false` otherwise.

-----------------------

## Notes:

### Memory resources

If delegate's target cannot be stored within a delegate, it must be allocated dynamically through the centralized global memory resource. If this memory allocation fails, the behavior depends on the installed memory resource. Custom memory resources may return a null pointer on failed memory allocation; default memory resource throws `std::bad_alloc`.

A custom memory resource can be installed through the experimental API - `vdk::memory::delegate::memory()` function that accepts and stores pointer to an instance of a class inherited from `vdk::memory::delegate::memory_resource`.

_Note!_ The memory resource can be installed only once. This avoids many problems that could otherwise arise from incompatible memory resources trying to deallocate each other's memory.

If memory allocation fails during delegate construction (by returning a null pointer), the constructed delegate instance will be empty. If memory allocation fails during assignment (either by returning a null pointer or by throwing an exception), delegate will remain unchanged.

If a delegate's target satisfies the following requirements, it is guaranteed that no dynamic memory allocation takes place:

1. The target's size and alignment are less than or equal to: `sizeof(ptr_to_any_callable) + sizeof(ptr_to_any_object)`.
2. The target's move operations are `noexcept`.