/*===================================================================
*	Copyright (c) Vadim Karkhin. All rights reserved.
*	Use, modification, and distribution is subject to license terms.
*	You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#ifndef VDK_DELEGATE_H
#define VDK_DELEGATE_H

namespace vdk
{

// TEMPLATE CLASS delegate
template<typename _ResT, typename ... _ArgTs>
class delegate;

template<typename _ResT, typename ... _ArgTs>
class delegate<_ResT(_ArgTs...)> final
{
	using byte			= unsigned char;
	using size_type		= unsigned int;
	using nullptr_type	= decltype(nullptr);

	// Structure to store target pointers
	template<typename _Class, typename _Signature>
	struct target
	{
		using function_ptr_t = _Signature;
		using instance_ptr_t = _Class*;
		function_ptr_t mp_function;
		instance_ptr_t mp_instance;
	};

	// Unknown default class (undefined)
	class default_class;

	// Unknown default function (undefined)
	using default_function = void(default_class::*)(void);

	// Default target type
	using default_type = target<default_class, default_function>;

	// Size of default target data
	static const size_type target_size = sizeof(default_type);

	// Storage for target data
	using storage_type = byte[target_size];

	// Type of invoker-function
	using invoker_type = _ResT(*)(const byte * const, _ArgTs&&...);

	// Remove reference
	template<typename _Ty> struct remove_reference       { using type = _Ty; };
	template<typename _Ty> struct remove_reference<_Ty&> { using type = _Ty; };
	template<typename _Ty> struct remove_reference<_Ty&&>{ using type = _Ty; };

	// Forward argument
	template<typename _Ty> inline static constexpr
	_Ty&& forward(typename remove_reference<_Ty>::type& arg) noexcept
	{ return (static_cast<_Ty&&>(arg)); }
	template<typename _Ty> inline static constexpr
	_Ty&& forward(typename remove_reference<_Ty>::type&& arg) noexcept
	{ return (static_cast<_Ty&&>(arg)); }

	// Invoke static method / free function
	template<nullptr_type, typename _Signature>
	static _ResT invoke(const byte * const data, _ArgTs&& ... args)
	{
		return
		(*reinterpret_cast<const target<nullptr_type, _Signature>*>
		(data)->mp_function)
		(forward<_ArgTs>(args)...);
	}

	// Invoke method
	template<typename _Class, typename _Signature>
	static _ResT invoke(const byte * const data, _ArgTs&& ... args)
	{
		return
		(reinterpret_cast<const target<_Class, _Signature>*>
		(data)->mp_instance->*
		reinterpret_cast<const target<_Class, _Signature>*>
		(data)->mp_function)
		(forward<_ArgTs>(args)...);
	}

	// Invoke function object (functor)
	template<typename _Class, nullptr_type>
	static _ResT invoke(const byte * const data, _ArgTs&& ... args)
	{
		return
		(*reinterpret_cast<const target<_Class, nullptr_type>*>
		(data)->mp_instance)
		(forward<_ArgTs>(args)...);
	}

	// Compare storages
	int compare(const byte * const storage) const noexcept
	{
		for (size_type index = 0; index < target_size; ++index)
		{
			if (mp_target[index] < storage[index])
				return -1;
			else
			if (mp_target[index] > storage[index])
				return +1;
		}
		return 0;
	}

public:

	// Construct empty delegate
	delegate() noexcept
	: mp_target{}, mp_invoker{ nullptr }
	{}

	// Construct empty delegate
	explicit delegate(nullptr_type) noexcept
	: delegate()
	{}

	// Construct delegate with static method / free function
	explicit delegate(_ResT(*function)(_ArgTs...)) noexcept
	: delegate()
	{
		using _Signature = decltype(function);
		auto storage =
		reinterpret_cast<target<nullptr_type, _Signature>*>(&mp_target[0]);
		storage->mp_function = function;
		storage->mp_instance = nullptr;
		mp_invoker = &delegate::invoke<nullptr, _Signature>;
	}

	// Construct delegate with method
	template<typename _Class, typename _Signature>
	delegate(_Class * object, _Signature method) noexcept
	: delegate()
	{
		auto storage =
		reinterpret_cast<target<_Class, _Signature>*>(&mp_target[0]);
		storage->mp_function = method;
		storage->mp_instance = object;
		mp_invoker = &delegate::invoke<_Class, _Signature>;
	}

	// Construct delegate with function object (functor)
	template<typename _Class>
	explicit delegate(_Class * functor) noexcept
	: delegate()
	{
		auto storage =
		reinterpret_cast<target<_Class, nullptr_type>*>(&mp_target[0]);
		storage->mp_function = nullptr;
		storage->mp_instance = functor;
		mp_invoker = &delegate::invoke<_Class, nullptr>;
	}

	// Copy-construct delegate
	delegate(const delegate & other) noexcept
	: delegate()
	{
		for (size_type index = 0; index < target_size; ++index)
		{
			mp_target[index] = other.mp_target[index];
		}
		mp_invoker = other.mp_invoker;
	}

	// Destroy delegate
	~delegate() noexcept
	{}

	// Copy-assign delegate
	delegate & operator=(const delegate & other) noexcept
	{
		if (this != &other)
		{
			for (size_type index = 0; index < target_size; ++index)
			{
				mp_target[index] = other.mp_target[index];
			}
			mp_invoker = other.mp_invoker;
		}
		return *this;
	}

	// Assign null pointer
	delegate & operator=(nullptr_type) noexcept
	{
		for (size_type index = 0; index < target_size; ++index)
		{
			mp_target[index] = 0;
		}
		mp_invoker = nullptr;
		return *this;
	}

	// Compare delegates (equal)
	bool operator==(const delegate & other) const noexcept
	{
		return compare(&other.mp_target[0]) == 0;
	}

	// Compare delegates (not equal)
	bool operator!=(const delegate & other) const noexcept
	{
		return compare(&other.mp_target[0]) != 0;
	}

	// Compare delegate to null pointer (equal)
	bool operator==(nullptr_type) const noexcept
	{
		return mp_invoker == nullptr;
	}

	// Compare delegate to null pointer (not equal)
	bool operator!=(nullptr_type) const noexcept
	{
		return mp_invoker != nullptr;
	}

	// Compare delegates (less)
	bool operator<(const delegate & other) const noexcept
	{
		return compare(&other.mp_target[0]) == -1;
	}

	// Compare delegates (less or equal)
	bool operator<=(const delegate & other) const noexcept
	{
		return	(compare(&other.mp_target[0]) == -1) ||
				(compare(&other.mp_target[0]) == 0);
	}

	// Compare delegates (greater)
	bool operator>(const delegate & other) const noexcept
	{
		return compare(&other.mp_target[0]) == +1;
	}

	// Compare delegates (greater or equal)
	bool operator>=(const delegate & other) const noexcept
	{
		return	(compare(&other.mp_target[0]) == +1) ||
				(compare(&other.mp_target[0]) == 0);
	}

	// Call delegate
	_ResT operator()(_ArgTs ... args) const
	{
		return (*mp_invoker)(&mp_target[0], forward<_ArgTs>(args)...);
	}

	// Check whether delegate is null
	explicit operator bool() const noexcept
	{
		return mp_invoker != nullptr;
	}

private:

	alignas(default_type)storage_type mp_target;
	alignas(invoker_type)invoker_type mp_invoker;

};

// Compare delegates (equal)
template<typename _ResT, typename ... _ArgTs>
bool operator==(const delegate<_ResT(_ArgTs...)> & lhs,
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return lhs == rhs;
}

// Compare delegates (not equal)
template<typename _ResT, typename ... _ArgTs>
bool operator!=(const delegate<_ResT(_ArgTs...)> & lhs,
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return lhs != rhs;
}

// Compare delegate with null pointer (equal)
template<typename _ResT, typename ... _ArgTs>
bool operator==(const delegate<_ResT(_ArgTs...)> & lhs,
				decltype(nullptr)) noexcept
{
	return lhs == nullptr;
}

// Compare delegate with null pointer (not equal)
template<typename _ResT, typename ... _ArgTs>
bool operator!=(const delegate<_ResT(_ArgTs...)> & lhs,
				decltype(nullptr)) noexcept
{
	return lhs != nullptr;
}

// Compare delegate with null pointer (equal)
template<typename _ResT, typename ... _ArgTs>
bool operator==(decltype(nullptr),
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return rhs == nullptr;
}

// Compare delegate with null pointer (not equal)
template<typename _ResT, typename ... _ArgTs>
bool operator!=(decltype(nullptr),
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return rhs != nullptr;
}

// Compare delegates (less)
template<typename _ResT, typename ... _ArgTs>
bool operator<( const delegate<_ResT(_ArgTs...)> & lhs,
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return lhs < rhs;
}

// Compare delegates (less or equal)
template<typename _ResT, typename ... _ArgTs>
bool operator<=(const delegate<_ResT(_ArgTs...)> & lhs,
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return lhs <= rhs;
}

// Compare delegates (greater)
template<typename _ResT, typename ... _ArgTs>
bool operator>( const delegate<_ResT(_ArgTs...)> & lhs,
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return lhs > rhs;
}

// Compare delegates (greater or equal)
template<typename _ResT, typename ... _ArgTs>
bool operator>=(const delegate<_ResT(_ArgTs...)> & lhs,
				const delegate<_ResT(_ArgTs...)> & rhs) noexcept
{
	return lhs >= rhs;
}

}

#endif