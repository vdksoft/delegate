/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#ifndef VDK_DELEGATE_H
#define VDK_DELEGATE_H

#include <new>
#include <cassert>
#include <cstddef>
#include <utility>
#include <type_traits>

namespace vdk
{
namespace memory::delegate
{
using std::size_t;

// Interface for memory resources
class memory_resource
{
public:
    virtual void * allocate(size_t size, size_t align) = 0;
    virtual void deallocate(void * addr, size_t size, size_t align) noexcept = 0;
    virtual ~memory_resource() noexcept = default;
};

// Default memory resource
class default_memory_resource : public memory_resource
{
public:
    void * allocate(size_t size, size_t align) override;
    void deallocate(void * addr, size_t size, size_t align) noexcept override;
    ~default_memory_resource() noexcept override = default;
};

inline void * default_memory_resource::
allocate(size_t size, size_t align)
{
    return ::operator new(size, std::align_val_t{ align });
}
inline void default_memory_resource::
deallocate(void * addr, size_t size, size_t align) noexcept
{
    ::operator delete(addr, size, std::align_val_t{ align });
}

// Global memory resource for all allocations | deallocations
inline memory_resource *
memory(memory_resource * r = nullptr) noexcept
{
    static memory_resource * const resource = r ? r :
        [] { static default_memory_resource dr{}; return &dr; }();
    return resource;
}

} // namespace memory::delegate

// Internal implementation details
namespace internal::delegate
{
using std::size_t;
using vdk::memory::delegate::memory;

// Helper class for memory management during object construction
class memory_owner
{
public:

    memory_owner(size_t size, size_t align);
    ~memory_owner() noexcept;

    void * get() const noexcept;
    void release() noexcept;

    memory_owner(const memory_owner &) = delete;
    memory_owner & operator=(const memory_owner &) = delete;

private:

    void * addr_;
    size_t size_;
    size_t align_;
};

inline memory_owner::memory_owner(size_t size, size_t align)
    : addr_{ memory()->allocate(size, align) },
      size_{ size },
      align_{ align }
{}

inline memory_owner::~memory_owner() noexcept
{
    if (addr_) memory()->deallocate(addr_, size_, align_);
}

inline void * memory_owner::get() const noexcept
{
    return addr_;
}

inline void memory_owner::release() noexcept
{
    addr_ = nullptr;
}

// Check whether T is equality comparable
template<typename T, typename = bool>
struct is_equality_comparable : std::false_type {};
template<typename T>
struct is_equality_comparable<T,
    decltype(std::declval<T&>() == std::declval<T&>())>
    : std::true_type {};
template<typename T> inline constexpr bool
is_equality_comparable_v = is_equality_comparable<T>::value;

// Structure to estimate size and alignment of internal buffer
struct block
{
    class undef;
    union
    {
        void(undef::*p1)();
        undef undef::*p2;
        void(*p3)();
    };
    undef*p4;
};

// Check whether T can be stored in internal buffer
template<typename T> inline constexpr
bool is_internal_v = sizeof(T) <= sizeof(block) &&
                     alignof(T) <= alignof(block) &&
                     std::is_nothrow_move_constructible_v<T>;

// Internal storage for delegate
struct storage
{
    void * get() noexcept;
    const void * get() const noexcept;
    volatile void * get() volatile noexcept;
    const volatile void * get() const volatile noexcept;

    template<typename T>
    T * get_as() noexcept;
    template<typename T>
    const T * get_as() const noexcept;
    template<typename T>
    volatile T * get_as() volatile noexcept;
    template<typename T>
    const volatile T * get_as() const volatile noexcept;

    std::aligned_storage_t<sizeof(block), alignof(block)> data_;
};

inline void * storage::get() noexcept
{
    return &data_;
}
inline const void * storage::get() const noexcept
{
    return &data_;
}
inline volatile void * storage::get() volatile noexcept
{
    return &data_;
}
inline const volatile void * storage::get() const volatile noexcept
{
    return &data_;
}
template<typename T>
inline T * storage::get_as() noexcept
{
    if constexpr(is_internal_v<T>)
        return static_cast<T*>(get());
    else
        return *static_cast<T**>(get());
}
template<typename T>
inline const T * storage::get_as() const noexcept
{
    if constexpr(is_internal_v<T>)
        return static_cast<const T*>(get());
    else
        return *static_cast<const T*const*>(get());
}
template<typename T>
inline volatile T * storage::get_as() volatile noexcept
{
    if constexpr(is_internal_v<T>)
        return static_cast<volatile T*>(get());
    else
        return *static_cast<volatile T*volatile*>(get());
}
template<typename T>
inline const volatile T * storage::get_as() const volatile noexcept
{
    if constexpr(is_internal_v<T>)
        return static_cast<const volatile T*>(get());
    else
        return *static_cast<const volatile T*const volatile*>(get());
}

// Artificial virtual table
struct vtbl
{
    void(*move)(storage&, storage&)noexcept;
    bool(*compare)(const storage&, const storage&)noexcept;
    void(*destroy)(storage&)noexcept;
};

// Move stored callable object
template<typename T>
inline void move(storage & src, storage & dst) noexcept
{
    if constexpr(is_internal_v<T>)
        ::new (dst.get()) T{ std::move(*src.template get_as<T>()) };
    else
        *dst.template get_as<T*>() = src.template get_as<T>();
}

// Compare stored callable objects
template<typename T>
inline bool compare([[maybe_unused]] const storage & lhs,
                    [[maybe_unused]] const storage & rhs) noexcept
{
    if constexpr(is_equality_comparable_v<T>)
        return *lhs.template get_as<T>() == *rhs.template get_as<T>();
    else
        return false;
}

// Destroy stored callable object
template<typename T>
inline void destroy(storage & self) noexcept
{
    if constexpr(is_internal_v<T>)
    {
        self.template get_as<T>()->~T();
    }
    else
    {
        auto p = self.template get_as<T>();
        p->~T();
        memory()->deallocate(p, sizeof(T), alignof(T));
    }
}

// Obtain virtual table for type T
template<typename T> inline
const vtbl * vtable() noexcept
{
    static constexpr vtbl tbl{ &move<T>, &compare<T>, &destroy<T> };
    return &tbl;
}

// Traits for supported delegate types
template<typename> struct traits;

#define VDK_NONE
#define VDK_TRAITS_CV_LR(CVQUAL, LRQUAL)\
template<typename R, typename ... A>\
struct traits<R(A...) CVQUAL LRQUAL> : storage\
{\
    R(*call_)(CVQUAL storage*, A&&...) {};\
\
    using fn_t = R(*)(A...);\
\
    template<typename T>\
    static constexpr bool is_invocable_v =\
        std::is_invocable_r_v<R, CVQUAL T LRQUAL, A...>;\
\
    template<typename T>\
    static R invoke(CVQUAL storage * self, A&& ... args)\
    {\
        return (*self->get_as<T>())(std::forward<A>(args)...);\
    }\
    inline R operator()(A ... args) CVQUAL LRQUAL\
    {\
        assert(call_ != nullptr);\
        return call_(this, std::forward<A>(args)...);\
    }\
};
#define VDK_TRAITS_CV_LR_NOEXCEPT(CVQUAL, LRQUAL)\
template<typename R, typename ... A>\
struct traits<R(A...) CVQUAL LRQUAL noexcept> : storage\
{\
    R(*call_)(CVQUAL storage*, A&&...)noexcept {};\
\
    using fn_t = R(*)(A...)noexcept;\
\
    template<typename T>\
    static constexpr bool is_invocable_v =\
        std::is_nothrow_invocable_r_v<R, CVQUAL T LRQUAL, A...>;\
\
    template<typename T>\
    static R invoke(CVQUAL storage * self, A&& ... args)noexcept\
    {\
        return (*self->get_as<T>())(std::forward<A>(args)...);\
    }\
    inline R operator()(A ... args) CVQUAL LRQUAL noexcept\
    {\
        assert(call_ != nullptr);\
        return call_(this, std::forward<A>(args)...);\
    }\
};
#define VDK_TRAITS_CV_RR(CVQUAL)\
template<typename R, typename ... A>\
struct traits<R(A...) CVQUAL &&> : storage\
{\
    R(*call_)(CVQUAL storage*, A&&...) {};\
\
    using fn_t = R(*)(A...);\
\
    template<typename T>\
    static constexpr bool is_invocable_v =\
        std::is_invocable_r_v<R, CVQUAL T &&, A...>;\
\
    template<typename T>\
    static R invoke(CVQUAL storage * self, A&& ... args)\
    {\
        return std::move(*self->get_as<T>())(std::forward<A>(args)...);\
    }\
    inline R operator()(A ... args) CVQUAL &&\
    {\
        assert(call_ != nullptr);\
        return call_(this, std::forward<A>(args)...);\
    }\
};
#define VDK_TRAITS_CV_RR_NOEXCEPT(CVQUAL)\
template<typename R, typename ... A>\
struct traits<R(A...) CVQUAL && noexcept> : storage\
{\
    R(*call_)(CVQUAL storage*, A&&...)noexcept {};\
\
    using fn_t = R(*)(A...)noexcept;\
\
    template<typename T>\
    static constexpr bool is_invocable_v =\
        std::is_nothrow_invocable_r_v<R, CVQUAL T &&, A...>;\
\
    template<typename T>\
    static R invoke(CVQUAL storage * self, A&& ... args)noexcept\
    {\
        return std::move(*self->get_as<T>())(std::forward<A>(args)...);\
    }\
    inline R operator()(A ... args) CVQUAL && noexcept\
    {\
        assert(call_ != nullptr);\
        return call_(this, std::forward<A>(args)...);\
    }\
};

VDK_TRAITS_CV_LR(VDK_NONE, VDK_NONE)
VDK_TRAITS_CV_LR(const, VDK_NONE)
VDK_TRAITS_CV_LR(volatile, VDK_NONE)
VDK_TRAITS_CV_LR(const volatile, VDK_NONE)
VDK_TRAITS_CV_LR(VDK_NONE, &)
VDK_TRAITS_CV_LR(const, &)
VDK_TRAITS_CV_LR(volatile, &)
VDK_TRAITS_CV_LR(const volatile, &)
VDK_TRAITS_CV_LR_NOEXCEPT(VDK_NONE, VDK_NONE)
VDK_TRAITS_CV_LR_NOEXCEPT(const, VDK_NONE)
VDK_TRAITS_CV_LR_NOEXCEPT(volatile, VDK_NONE)
VDK_TRAITS_CV_LR_NOEXCEPT(const volatile, VDK_NONE)
VDK_TRAITS_CV_LR_NOEXCEPT(VDK_NONE, &)
VDK_TRAITS_CV_LR_NOEXCEPT(const, &)
VDK_TRAITS_CV_LR_NOEXCEPT(volatile, &)
VDK_TRAITS_CV_LR_NOEXCEPT(const volatile, &)
VDK_TRAITS_CV_RR(VDK_NONE)
VDK_TRAITS_CV_RR(const)
VDK_TRAITS_CV_RR(volatile)
VDK_TRAITS_CV_RR(const volatile)
VDK_TRAITS_CV_RR_NOEXCEPT(VDK_NONE)
VDK_TRAITS_CV_RR_NOEXCEPT(const)
VDK_TRAITS_CV_RR_NOEXCEPT(volatile)
VDK_TRAITS_CV_RR_NOEXCEPT(const volatile)

#undef VDK_NONE
#undef VDK_TRAITS_CV_LR
#undef VDK_TRAITS_CV_LR_NOEXCEPT
#undef VDK_TRAITS_CV_RR
#undef VDK_TRAITS_CV_RR_NOEXCEPT

} // namespace internal::delegate

// Public delegate interface
template<typename Fn>
class delegate final : internal::delegate::traits<Fn>
{
    using base = internal::delegate::traits<Fn>;
    using vtbl = internal::delegate::vtbl;

    using base::call_;
    using typename base::fn_t;

    template<typename T> static constexpr
        bool is_internal_v = internal::delegate::is_internal_v<T>;
    template<typename T> static constexpr
        bool is_invocable_v = base::template is_invocable_v<T>;

public:

    delegate() noexcept = default;
    delegate(std::nullptr_t) noexcept {}
    delegate(fn_t func) noexcept;

    template<typename F, typename =
        std::enable_if_t<is_invocable_v<F>>>
    delegate(F func) noexcept(is_internal_v<F>);

    delegate(delegate && other) noexcept;
    delegate & operator=(delegate && other) noexcept;
    delegate & operator=(std::nullptr_t) noexcept;
    delegate & operator=(fn_t func) noexcept;

    template<typename F, typename =
        std::enable_if_t<is_invocable_v<F>>>
    delegate & operator=(F func) noexcept(is_internal_v<F>);

    ~delegate() noexcept;

    using base::operator();
    explicit operator bool() const noexcept;

    bool operator==(const delegate & other) const noexcept;
    bool operator!=(const delegate & other) const noexcept;

    delegate(const delegate &) = delete;
    delegate & operator=(const delegate &) = delete;

private:

    const vtbl * vptr_{};
};

template<typename Fn>
delegate<Fn>::delegate(fn_t func) noexcept
{
    if (!func) return;
    *(base::template get_as<fn_t>()) = func;
    call_ = &base::template invoke<fn_t>;
    vptr_ = internal::delegate::vtable<fn_t>();
}

template<typename Fn>
template<typename F, typename>
delegate<Fn>::delegate(F func) noexcept(is_internal_v<F>)
{
    if constexpr(std::is_pointer_v<F>)
        if (!func) return;

    if constexpr(is_internal_v<F>)
    {
        ::new (base::get()) F{ std::move(func) };
    }
    else
    {
        internal::delegate::memory_owner mem{ sizeof(F), alignof(F) };

        if (mem.get())
        {
            *(base::template get_as<F*>()) =
                ::new (mem.get()) F{ std::move(func) };
            mem.release();
        }
        else
            return;
    }

    call_ = &base::template invoke<F>;
    vptr_ = internal::delegate::vtable<F>();
}

template<typename Fn>
delegate<Fn>::delegate(delegate && other) noexcept
{
    call_ = other.call_;
    vptr_ = other.vptr_;
    if (vptr_) vptr_->move(other, *this);
    other.call_ = nullptr;
    other.vptr_ = nullptr;
}

template<typename Fn>
delegate<Fn> & delegate<Fn>::operator=(delegate && other) noexcept
{
    if (this != &other)
    {
        if (vptr_) vptr_->destroy(*this);
        call_ = other.call_;
        vptr_ = other.vptr_;
        if (vptr_) vptr_->move(other, *this);
        other.call_ = nullptr;
        other.vptr_ = nullptr;
    }
    return *this;
}

template<typename Fn>
delegate<Fn> & delegate<Fn>::operator=(std::nullptr_t) noexcept
{
    if (vptr_) vptr_->destroy(*this);
    call_ = nullptr;
    vptr_ = nullptr;
    return *this;
}

template<typename Fn>
delegate<Fn> & delegate<Fn>::operator=(fn_t func) noexcept
{
    if (!func) return *this;
    if (vptr_) vptr_->destroy(*this);
    *(base::template get_as<fn_t>()) = func;
    call_ = &base::template invoke<fn_t>;
    vptr_ = internal::delegate::vtable<fn_t>();
    return *this;
}

template<typename Fn>
template<typename F, typename>
delegate<Fn> & delegate<Fn>::operator=(F func) noexcept(is_internal_v<F>)
{
    if constexpr(std::is_pointer_v<F>)
        if (!func) return *this;

    if constexpr(is_internal_v<F>)
    {
        if (vptr_) vptr_->destroy(*this);
        ::new (base::get()) F{ std::move(func) };
    }
    else
    {
        internal::delegate::memory_owner mem{ sizeof(F), alignof(F) };

        if (mem.get())
        {
            auto addr = ::new (mem.get()) F{ std::move(func) };
            if (vptr_) vptr_->destroy(*this);
            *(base::template get_as<F*>()) = addr;
            mem.release();
        }
        else
            return *this;
    }

    call_ = &base::template invoke<F>;
    vptr_ = internal::delegate::vtable<F>();
    return *this;
}

template<typename Fn>
delegate<Fn>::~delegate() noexcept
{
    if (vptr_) vptr_->destroy(*this);
}

template<typename Fn>
delegate<Fn>::operator bool() const noexcept
{
    return vptr_ != nullptr;
}

template<typename Fn>
bool delegate<Fn>::operator==(const delegate & other) const noexcept
{
    if (!vptr_ && !other.vptr_)
        return true;
    if (vptr_ != other.vptr_)
        return false;
    return vptr_->compare(*this, other);
}

template<typename Fn>
bool delegate<Fn>::operator!=(const delegate & other) const noexcept
{
    return !(*this == other);
}

template<typename Fn>
bool operator==(const delegate<Fn> & lhs, std::nullptr_t) noexcept
{
    return !lhs;
}

template<typename Fn>
bool operator==(std::nullptr_t, const delegate<Fn> & rhs) noexcept
{
    return !rhs;
}

template<typename Fn>
bool operator!=(const delegate<Fn> & lhs, std::nullptr_t) noexcept
{
    return static_cast<bool>(lhs);
}

template<typename Fn>
bool operator!=(std::nullptr_t, const delegate<Fn> & rhs) noexcept
{
    return static_cast<bool>(rhs);
}

} // namespace vdk

#endif // VDK_DELEGATE_H