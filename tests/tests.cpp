/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#include <string>
#include <memory>
#include <utility>
#include <stdexcept>
#include <functional>
#include <type_traits>

#include <gtest/gtest.h>
#include <delegate.h>

namespace
{
// Supported types
using fn_01 = int(int);
using fn_02 = int(int)const;
using fn_03 = int(int)volatile;
using fn_04 = int(int)const volatile;
using fn_05 = int(int)&;
using fn_06 = int(int)const&;
using fn_07 = int(int)volatile&;
using fn_08 = int(int)const volatile&;
using fn_09 = int(int)&&;
using fn_10 = int(int)const&&;
using fn_11 = int(int)volatile&&;
using fn_12 = int(int)const volatile&&;
using fn_13 = int(int) noexcept;
using fn_14 = int(int)const noexcept;
using fn_15 = int(int)volatile noexcept;
using fn_16 = int(int)const volatile noexcept;
using fn_17 = int(int)& noexcept;
using fn_18 = int(int)const& noexcept;
using fn_19 = int(int)volatile& noexcept;
using fn_20 = int(int)const volatile& noexcept;
using fn_21 = int(int)&& noexcept;
using fn_22 = int(int)const&& noexcept;
using fn_23 = int(int)volatile&& noexcept;
using fn_24 = int(int)const volatile&& noexcept;

// Test-values for supported types
constexpr int fn_01_v = 1;
constexpr int fn_02_v = 2;
constexpr int fn_03_v = 3;
constexpr int fn_04_v = 4;
constexpr int fn_05_v = 5;
constexpr int fn_06_v = 6;
constexpr int fn_07_v = 7;
constexpr int fn_08_v = 8;
constexpr int fn_09_v = 9;
constexpr int fn_10_v = 10;
constexpr int fn_11_v = 11;
constexpr int fn_12_v = 12;
constexpr int fn_13_v = 13;
constexpr int fn_14_v = 14;
constexpr int fn_15_v = 15;
constexpr int fn_16_v = 16;
constexpr int fn_17_v = 17;
constexpr int fn_18_v = 18;
constexpr int fn_19_v = 19;
constexpr int fn_20_v = 20;
constexpr int fn_21_v = 21;
constexpr int fn_22_v = 22;
constexpr int fn_23_v = 23;
constexpr int fn_24_v = 24;

// ====================== Test-utilities ====================== //

struct test_exception : std::runtime_error
{
    test_exception()
        : std::runtime_error{ "dummy_exception" }
    {}
};

struct test_class
{
    int method(int arg)
    {
        return fn_01_v + arg;
    }
};

// ===================== Function objects ===================== //

struct functor_base
{
    explicit functor_base(int arg) noexcept
        : value_{ arg }
    {}
    bool operator==(const functor_base & other) const noexcept
    {
        return value_ == other.value_;
    }
    int value_;
};

struct functor_base_except : functor_base
{
    using functor_base::functor_base;
    using functor_base::operator==;

    int operator()(int arg)
    {
        return arg + fn_01_v;
    }
    int operator()(int arg)const
    {
        return arg + fn_02_v;
    }
    int operator()(int arg)volatile
    {
        return arg + fn_03_v;
    }
    int operator()(int arg)const volatile
    {
        return arg + fn_04_v;
    }
};

struct functor_base_ref_except : functor_base
{
    using functor_base::functor_base;
    using functor_base::operator==;

    int operator()(int arg)&
    {
        return arg + fn_05_v;
    }
    int operator()(int arg)const&
    {
        return arg + fn_06_v;
    }
    int operator()(int arg)volatile&
    {
        return arg + fn_07_v;
    }
    int operator()(int arg)const volatile&
    {
        return arg + fn_08_v;
    }
    int operator()(int arg) &&
    {
        return arg + fn_09_v;
    }
    int operator()(int arg)const&&
    {
        return arg + fn_10_v;
    }
    int operator()(int arg)volatile&&
    {
        return arg + fn_11_v;
    }
    int operator()(int arg)const volatile&&
    {
        return arg + fn_12_v;
    }
};

struct functor_base_noexcept : functor_base
{
    using functor_base::functor_base;
    using functor_base::operator==;

    int operator()(int arg) noexcept
    {
        return arg + fn_13_v;
    }
    int operator()(int arg)const noexcept
    {
        return arg + fn_14_v;
    }
    int operator()(int arg)volatile noexcept
    {
        return arg + fn_15_v;
    }
    int operator()(int arg)const volatile noexcept
    {
        return arg + fn_16_v;
    }
};

struct functor_base_ref_noexcept : functor_base
{
    using functor_base::functor_base;
    using functor_base::operator==;

    int operator()(int arg)& noexcept
    {
        return arg + fn_17_v;
    }
    int operator()(int arg)const& noexcept
    {
        return arg + fn_18_v;
    }
    int operator()(int arg)volatile& noexcept
    {
        return arg + fn_19_v;
    }
    int operator()(int arg)const volatile& noexcept
    {
        return arg + fn_20_v;
    }
    int operator()(int arg) && noexcept
    {
        return arg + fn_21_v;
    }
    int operator()(int arg)const&& noexcept
    {
        return arg + fn_22_v;
    }
    int operator()(int arg)volatile&& noexcept
    {
        return arg + fn_23_v;
    }
    int operator()(int arg)const volatile&& noexcept
    {
        return arg + fn_24_v;
    }
};

// Types to test internally and externally constructed targets
using small_t = struct {};
using large_t = std::aligned_storage_t<sizeof(vdk::delegate<void(void)>)>;

template<typename DataType>
struct functor_except : functor_base_except
{
    using functor_base_except::functor_base_except;
    using functor_base_except::operator();
    DataType data_;
};

template<typename DataType>
struct functor_ref_except : functor_base_ref_except
{
    using functor_base_ref_except::functor_base_ref_except;
    using functor_base_ref_except::operator();
    DataType data_;
};

template<typename DataType>
struct functor_noexcept : functor_base_noexcept
{
    using functor_base_noexcept::functor_base_noexcept;
    using functor_base_noexcept::operator();
    DataType data_;
};

template<typename DataType>
struct functor_ref_noexcept : functor_base_ref_noexcept
{
    using functor_base_ref_noexcept::functor_base_ref_noexcept;
    using functor_base_ref_noexcept::operator();
    DataType data_;
};

struct functor_no_comparison
{
    int operator()(int) noexcept { return {}; }
    int operator()(int)const noexcept { return {}; }
    int operator()(int)volatile noexcept { return {}; }
    int operator()(int)const volatile noexcept { return {}; }
    large_t data_;
};

struct functor_no_copy_move
{
    functor_no_copy_move() noexcept = default;
    functor_no_copy_move(const functor_no_copy_move &) = delete;
    functor_no_copy_move & operator=(const functor_no_copy_move &) = delete;
    functor_no_copy_move(functor_no_copy_move &&) = delete;
    functor_no_copy_move & operator=(functor_no_copy_move &&) = delete;
    int operator()(int arg) { return fn_01_v + arg; }
    int operator()(int arg)const { return fn_02_v + arg; }
};

struct functor_throw_on_move
{
    functor_throw_on_move() noexcept = default;
    functor_throw_on_move(const functor_throw_on_move &) noexcept = default;
    functor_throw_on_move & operator=(const functor_throw_on_move &) noexcept = default;
    functor_throw_on_move(functor_throw_on_move &&)
    {
        throw test_exception{};
    }
    functor_throw_on_move & operator=(functor_throw_on_move &&)
    {
        throw test_exception{};
    }
    int operator()(int arg) { return arg; }
};

struct functor_throw_on_call
{
    explicit functor_throw_on_call(int arg) noexcept
        : data_{ arg }
    {}
    functor_throw_on_call(const functor_throw_on_call & other) noexcept
        : data_{ other.data_ }
    {}
    functor_throw_on_call & operator=(const functor_throw_on_call & other) noexcept
    {
        data_ = other.data_;
        return *this;
    }
    bool operator==(const functor_throw_on_call & other) const noexcept
    {
        return data_ == other.data_;
    }
    int operator()(int)
    {
        throw test_exception{};
    }
    int data_ = 0;
};

// ========================= Functions ========================= //

int function_except(int arg)
{
    return arg + fn_01_v;
}

int function_noexcept(int arg) noexcept
{
    return arg + fn_13_v;
}

int function_unique(int arg) noexcept
{
    return arg;
}

int function_overload(int arg1, int arg2)
{
    return arg1 + arg2;
}

std::string function_overload(int arg)
{
    return std::to_string(arg);
}

} // namespace

// Fixture to test all common delegate's operations
template<typename T>
class DelegatesTest : public testing::Test
{
protected:
    using delegate_t = T;
};

using vdk::delegate;

using Types = testing::Types<
delegate<fn_01>, delegate<fn_02>, delegate<fn_03>,
delegate<fn_04>, delegate<fn_05>, delegate<fn_06>,
delegate<fn_07>, delegate<fn_08>, delegate<fn_09>,
delegate<fn_10>, delegate<fn_11>, delegate<fn_12>,
delegate<fn_13>, delegate<fn_14>, delegate<fn_15>,
delegate<fn_16>, delegate<fn_17>, delegate<fn_18>,
delegate<fn_19>, delegate<fn_20>, delegate<fn_21>,
delegate<fn_22>, delegate<fn_23>, delegate<fn_24>>;

TYPED_TEST_CASE(DelegatesTest, Types);

TYPED_TEST(DelegatesTest, Empty)
{
    using delegate_t = typename TestFixture::delegate_t;

    // Default construction
    delegate_t fn0;
    EXPECT_FALSE(fn0);
    EXPECT_EQ(fn0, nullptr);
    EXPECT_EQ(nullptr, fn0);

    // Move construction
    delegate_t fn1;
    delegate_t fn2{ std::move(fn1) };
    EXPECT_FALSE(fn1);
    EXPECT_FALSE(fn2);
    EXPECT_EQ(fn2, nullptr);
    EXPECT_EQ(nullptr, fn2);
    EXPECT_EQ(fn2, fn0);
    EXPECT_EQ(fn0, fn2);

    // Construction (null pointer)
    delegate_t fn3{ nullptr };
    EXPECT_FALSE(fn3);
    EXPECT_EQ(fn3, nullptr);
    EXPECT_EQ(nullptr, fn3);
    EXPECT_EQ(fn3, fn0);
    EXPECT_EQ(fn0, fn3);

    // Construction (function pointer which is null pointer)
    int(*func_ptr_null)(int)noexcept = nullptr;
    delegate_t fn4{ func_ptr_null };
    EXPECT_FALSE(fn4);
    EXPECT_EQ(fn4, nullptr);
    EXPECT_EQ(nullptr, fn4);
    EXPECT_EQ(fn4, fn0);
    EXPECT_EQ(fn0, fn4);

    // Move assignment
    delegate_t fn5;
    delegate_t fn6;
    fn5 = std::move(fn6);
    EXPECT_FALSE(fn5);
    EXPECT_EQ(fn5, nullptr);
    EXPECT_EQ(nullptr, fn5);
    EXPECT_EQ(fn5, fn0);
    EXPECT_EQ(fn0, fn5);

    // Assignment (null pointer)
    delegate_t fn7{ function_noexcept };
    fn7 = nullptr;
    EXPECT_FALSE(fn7);
    EXPECT_EQ(fn7, nullptr);
    EXPECT_EQ(nullptr, fn7);
    EXPECT_EQ(fn7, fn0);
    EXPECT_EQ(fn0, fn7);

    // Assignment (function pointer which is null pointer)
    delegate_t fn8;
    fn8 = func_ptr_null;
    EXPECT_FALSE(fn8);
    EXPECT_EQ(fn8, nullptr);
    EXPECT_EQ(nullptr, fn8);
    EXPECT_EQ(fn8, fn0);
    EXPECT_EQ(fn0, fn8);
}

TYPED_TEST(DelegatesTest, Function)
{
    using delegate_t = typename TestFixture::delegate_t;

    // Empty delegate
    delegate_t fn0;

    // Construction (function)
    delegate_t fn1{ function_noexcept };
    EXPECT_TRUE(fn1);
    EXPECT_NE(fn1, nullptr);
    EXPECT_NE(nullptr, fn1);
    EXPECT_NE(fn1, fn0);
    EXPECT_NE(fn0, fn1);

    // Move construction
    delegate_t fn2{ function_noexcept };
    delegate_t fn3{ std::move(fn2) };
    EXPECT_FALSE(fn2);
    EXPECT_TRUE(fn3);
    EXPECT_NE(fn3, nullptr);
    EXPECT_NE(nullptr, fn3);
    EXPECT_NE(fn3, fn0);
    EXPECT_NE(fn0, fn3);
    EXPECT_EQ(fn3, fn1);
    EXPECT_EQ(fn1, fn3);

    // Construction (function pointer)
    int(*func_ptr)(int)noexcept = function_noexcept;
    delegate_t fn4{ func_ptr };
    EXPECT_TRUE(fn4);
    EXPECT_NE(fn4, nullptr);
    EXPECT_NE(nullptr, fn4);
    EXPECT_NE(fn4, fn0);
    EXPECT_NE(fn0, fn4);
    EXPECT_EQ(fn4, fn1);
    EXPECT_EQ(fn1, fn4);

    // Move assignment
    delegate_t fn5;
    delegate_t fn6{ function_noexcept };
    fn5 = std::move(fn6);
    EXPECT_TRUE(fn5);
    EXPECT_NE(fn5, nullptr);
    EXPECT_NE(nullptr, fn5);
    EXPECT_NE(fn5, fn0);
    EXPECT_NE(fn0, fn5);
    EXPECT_EQ(fn5, fn1);
    EXPECT_EQ(fn1, fn5);

    // Assignment (function)
    delegate_t fn7;
    fn7 = function_noexcept;
    EXPECT_TRUE(fn7);
    EXPECT_NE(fn7, nullptr);
    EXPECT_NE(nullptr, fn7);
    EXPECT_NE(fn7, fn0);
    EXPECT_NE(fn0, fn7);
    EXPECT_EQ(fn7, fn1);
    EXPECT_EQ(fn1, fn7);

    // Assignment (function pointer)
    delegate_t fn8;
    fn8 = func_ptr;
    EXPECT_TRUE(fn8);
    EXPECT_NE(fn8, nullptr);
    EXPECT_NE(nullptr, fn8);
    EXPECT_NE(fn8, fn0);
    EXPECT_NE(fn0, fn8);
    EXPECT_EQ(fn8, fn1);
    EXPECT_EQ(fn1, fn8);

    // Assignment (function pointer which is null pointer)
    int(*func_ptr_null)(int)noexcept = nullptr;
    delegate_t fn9{ function_noexcept };
    fn9 = func_ptr_null;
    EXPECT_TRUE(fn9);
    EXPECT_NE(fn9, nullptr);
    EXPECT_NE(nullptr, fn9);
    EXPECT_NE(fn9, fn0);
    EXPECT_NE(fn0, fn9);
    EXPECT_EQ(fn9, fn1);
    EXPECT_EQ(fn1, fn9);
}

TYPED_TEST(DelegatesTest, SmallFunctor)
{
    using delegate_t = typename TestFixture::delegate_t;

    // Empty delegate
    delegate_t fn0;

    functor_noexcept<small_t> functor{ 10 };

    // Construction (functor l-value)
    delegate_t fn1{ functor };
    EXPECT_TRUE(fn1);
    EXPECT_NE(fn1, nullptr);
    EXPECT_NE(nullptr, fn1);
    EXPECT_NE(fn1, fn0);
    EXPECT_NE(fn0, fn1);

    // Construction (functor r-value)
    delegate_t fn2{ functor_noexcept<small_t>{ 10 } };
    EXPECT_TRUE(fn2);
    EXPECT_NE(fn2, nullptr);
    EXPECT_NE(nullptr, fn2);
    EXPECT_NE(fn2, fn0);
    EXPECT_NE(fn0, fn2);
    EXPECT_EQ(fn2, fn1);
    EXPECT_EQ(fn1, fn2);

    // Move construction
    delegate_t fn3{ functor };
    delegate_t fn4{ std::move(fn3) };
    EXPECT_FALSE(fn3);
    EXPECT_TRUE(fn4);
    EXPECT_NE(fn4, nullptr);
    EXPECT_NE(nullptr, fn4);
    EXPECT_NE(fn4, fn0);
    EXPECT_NE(fn0, fn4);
    EXPECT_EQ(fn4, fn1);
    EXPECT_EQ(fn1, fn4);

    // Move assignment
    delegate_t fn5;
    delegate_t fn6{ functor };
    fn5 = std::move(fn6);
    EXPECT_TRUE(fn5);
    EXPECT_NE(fn5, nullptr);
    EXPECT_NE(nullptr, fn5);
    EXPECT_NE(fn5, fn0);
    EXPECT_NE(fn0, fn5);
    EXPECT_EQ(fn5, fn1);
    EXPECT_EQ(fn1, fn5);

    // Assignment (functor l-value)
    delegate_t fn7;
    fn7 = functor;
    EXPECT_TRUE(fn7);
    EXPECT_NE(fn7, nullptr);
    EXPECT_NE(nullptr, fn7);
    EXPECT_NE(fn7, fn0);
    EXPECT_NE(fn0, fn7);
    EXPECT_EQ(fn7, fn1);
    EXPECT_EQ(fn1, fn7);

    // Assignment (functor r-value)
    delegate_t fn8;
    fn8 = functor_noexcept<small_t>{ 10 };
    EXPECT_TRUE(fn8);
    EXPECT_NE(fn8, nullptr);
    EXPECT_NE(nullptr, fn8);
    EXPECT_NE(fn8, fn0);
    EXPECT_NE(fn0, fn8);
    EXPECT_EQ(fn8, fn1);
    EXPECT_EQ(fn1, fn8);

    // Assignment (function pointer which is null pointer)
    int(*func_ptr_null)(int)noexcept = nullptr;
    delegate_t fn9{ functor_noexcept<small_t>{ 10 } };
    fn9 = func_ptr_null;
    EXPECT_TRUE(fn9);
    EXPECT_NE(fn9, nullptr);
    EXPECT_NE(nullptr, fn9);
    EXPECT_NE(fn9, fn0);
    EXPECT_NE(fn0, fn9);
    EXPECT_EQ(fn9, fn1);
    EXPECT_EQ(fn1, fn9);
}

TYPED_TEST(DelegatesTest, LargeFunctor)
{
    using delegate_t = typename TestFixture::delegate_t;

    // Empty delegate
    delegate_t fn0;

    functor_noexcept<large_t> functor{ 10 };

    // Construction (functor l-value)
    delegate_t fn1{ functor };
    EXPECT_TRUE(fn1);
    EXPECT_NE(fn1, nullptr);
    EXPECT_NE(nullptr, fn1);
    EXPECT_NE(fn1, fn0);
    EXPECT_NE(fn0, fn1);

    // Construction (functor r-value)
    delegate_t fn2{ functor_noexcept<large_t>{ 10 } };
    EXPECT_TRUE(fn2);
    EXPECT_NE(fn2, nullptr);
    EXPECT_NE(nullptr, fn2);
    EXPECT_NE(fn2, fn0);
    EXPECT_NE(fn0, fn2);
    EXPECT_EQ(fn2, fn1);
    EXPECT_EQ(fn1, fn2);

    // Move construction
    delegate_t fn3{ functor };
    delegate_t fn4{ std::move(fn3) };
    EXPECT_FALSE(fn3);
    EXPECT_TRUE(fn4);
    EXPECT_NE(fn4, nullptr);
    EXPECT_NE(nullptr, fn4);
    EXPECT_NE(fn4, fn0);
    EXPECT_NE(fn0, fn4);
    EXPECT_EQ(fn4, fn1);
    EXPECT_EQ(fn1, fn4);

    // Move assignment
    delegate_t fn5;
    delegate_t fn6{ functor };
    fn5 = std::move(fn6);
    EXPECT_TRUE(fn5);
    EXPECT_NE(fn5, nullptr);
    EXPECT_NE(nullptr, fn5);
    EXPECT_NE(fn5, fn0);
    EXPECT_NE(fn0, fn5);
    EXPECT_EQ(fn5, fn1);
    EXPECT_EQ(fn1, fn5);

    // Assignment (functor l-value)
    delegate_t fn7;
    fn7 = functor;
    EXPECT_TRUE(fn7);
    EXPECT_NE(fn7, nullptr);
    EXPECT_NE(nullptr, fn7);
    EXPECT_NE(fn7, fn0);
    EXPECT_NE(fn0, fn7);
    EXPECT_EQ(fn7, fn1);
    EXPECT_EQ(fn1, fn7);

    // Assignment (functor r-value)
    delegate_t fn8;
    fn8 = functor_noexcept<large_t>{ 10 };
    EXPECT_TRUE(fn8);
    EXPECT_NE(fn8, nullptr);
    EXPECT_NE(nullptr, fn8);
    EXPECT_NE(fn8, fn0);
    EXPECT_NE(fn0, fn8);
    EXPECT_EQ(fn8, fn1);
    EXPECT_EQ(fn1, fn8);

    // Assignment (function pointer which is null pointer)
    int(*func_ptr_null)(int)noexcept = nullptr;
    delegate_t fn9{ functor_noexcept<large_t>{ 10 } };
    fn9 = func_ptr_null;
    EXPECT_TRUE(fn9);
    EXPECT_NE(fn9, nullptr);
    EXPECT_NE(nullptr, fn9);
    EXPECT_NE(fn9, fn0);
    EXPECT_NE(fn0, fn9);
    EXPECT_EQ(fn9, fn1);
    EXPECT_EQ(fn1, fn9);
}

TYPED_TEST(DelegatesTest, Compare)
{
    using delegate_t = typename TestFixture::delegate_t;

    // Functors of different types
    delegate_t fn1{ functor_noexcept<small_t>{ 10 } };
    delegate_t fn2{ functor_noexcept<large_t>{ 10 } };
    EXPECT_NE(fn1, fn2);
    EXPECT_NE(fn2, fn1);

    // Small functors of the same type (equal)
    delegate_t fn3{ functor_noexcept<small_t>{ 10 } };
    delegate_t fn4{ functor_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn3, fn4);
    EXPECT_EQ(fn4, fn3);

    // Large functors of the same type (equal)
    delegate_t fn5{ functor_noexcept<large_t>{ 10 } };
    delegate_t fn6{ functor_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn5, fn6);
    EXPECT_EQ(fn6, fn5);

    // Functors without comparison operator
    delegate_t fn7{ functor_no_comparison{} };
    delegate_t fn8{ functor_no_comparison{} };
    EXPECT_NE(fn7, fn8);
    EXPECT_NE(fn8, fn7);

    // Functors and functions
    delegate_t fn9{ functor_no_comparison{} };
    delegate_t fn10{ functor_noexcept<small_t>{ 10 } };
    delegate_t fn11{ functor_noexcept<large_t>{ 10 } };
    delegate_t fn12{ function_noexcept };
    EXPECT_NE(fn9, fn10);
    EXPECT_NE(fn10, fn9);
    EXPECT_NE(fn9, fn11);
    EXPECT_NE(fn11, fn9);
    EXPECT_NE(fn9, fn12);
    EXPECT_NE(fn12, fn9);
    EXPECT_NE(fn10, fn11);
    EXPECT_NE(fn11, fn10);
    EXPECT_NE(fn10, fn12);
    EXPECT_NE(fn12, fn10);
    EXPECT_NE(fn11, fn12);
    EXPECT_NE(fn12, fn11);

    // Small functors of the same type (not equal)
    delegate_t fn13{ functor_noexcept<small_t>{ 1 } };
    delegate_t fn14{ functor_noexcept<small_t>{ 2 } };
    EXPECT_NE(fn13, fn14);
    EXPECT_NE(fn14, fn13);

    // Large functors of the same type (not equal)
    delegate_t fn15{ functor_noexcept<large_t>{ 1 } };
    delegate_t fn16{ functor_noexcept<large_t>{ 2 } };
    EXPECT_NE(fn15, fn16);
    EXPECT_NE(fn16, fn15);

    // Same function
    delegate_t fn17{ function_noexcept };
    delegate_t fn18{ function_noexcept };
    EXPECT_EQ(fn17, fn18);
    EXPECT_EQ(fn18, fn17);

    // Different functions
    delegate_t fn19{ function_noexcept };
    delegate_t fn20{ function_unique };
    EXPECT_NE(fn19, fn20);
    EXPECT_NE(fn20, fn19);
}

TYPED_TEST(DelegatesTest, Swap)
{
    using delegate_t = typename TestFixture::delegate_t;

    // Small functors
    delegate_t fn1{ functor_noexcept<small_t>{ 1 } };
    delegate_t fn2{ functor_noexcept<small_t>{ 2 } };
    std::swap(fn1, fn2);
    EXPECT_EQ(fn1, delegate_t{ functor_noexcept<small_t>{ 2 } });
    EXPECT_EQ(fn2, delegate_t{ functor_noexcept<small_t>{ 1 } });

    // Large functors
    delegate_t fn3{ functor_noexcept<large_t>{ 1 } };
    delegate_t fn4{ functor_noexcept<large_t>{ 2 } };
    std::swap(fn3, fn4);
    EXPECT_EQ(fn3, delegate_t{ functor_noexcept<large_t>{ 2 } });
    EXPECT_EQ(fn4, delegate_t{ functor_noexcept<large_t>{ 1 } });

    // Mixed functors
    delegate_t fn5{ functor_noexcept<small_t>{ 1 } };
    delegate_t fn6{ functor_noexcept<large_t>{ 2 } };
    std::swap(fn5, fn6);
    EXPECT_EQ(fn5, delegate_t{ functor_noexcept<large_t>{ 2 } });
    EXPECT_EQ(fn6, delegate_t{ functor_noexcept<small_t>{ 1 } });

    // Empty and small functor
    delegate_t fn7;
    delegate_t fn8{ functor_noexcept<small_t>{ 1 } };
    std::swap(fn7, fn8);
    EXPECT_TRUE(fn7);
    EXPECT_EQ(fn7, delegate_t{ functor_noexcept<small_t>{ 1 } });
    EXPECT_FALSE(fn8);

    // Empty and large functor
    delegate_t fn9;
    delegate_t fn10{ functor_noexcept<large_t>{ 2 } };
    std::swap(fn9, fn10);
    EXPECT_TRUE(fn9);
    EXPECT_EQ(fn9, delegate_t{ functor_noexcept<large_t>{ 2 } });
    EXPECT_FALSE(fn10);
}

TEST(DelegateTest, CallFunction)
{
    int arg = 10;

    delegate<fn_01> fn01{ function_except };
    EXPECT_EQ(fn01(arg), fn_01_v + arg);
    delegate<fn_13> fn02{ function_noexcept };
    EXPECT_EQ(fn02(arg), fn_13_v + arg);
}

TEST(DelegateTest, CallSmallFunctor)
{
    int arg = 10;

    delegate<fn_01> fn01{ functor_except<small_t>{ 10 } };
    EXPECT_EQ(fn01(arg), fn_01_v + arg);
    delegate<fn_02> fn02{ functor_except<small_t>{ 10 } };
    EXPECT_EQ(fn02(arg), fn_02_v + arg);
    delegate<fn_03> fn03{ functor_except<small_t>{ 10 } };
    EXPECT_EQ(fn03(arg), fn_03_v + arg);
    delegate<fn_04> fn04{ functor_except<small_t>{ 10 } };
    EXPECT_EQ(fn04(arg), fn_04_v + arg);
    delegate<fn_05> fn05{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(fn05(arg), fn_05_v + arg);
    delegate<fn_06> fn06{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(fn06(arg), fn_06_v + arg);
    delegate<fn_07> fn07{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(fn07(arg), fn_07_v + arg);
    delegate<fn_08> fn08{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(fn08(arg), fn_08_v + arg);
    delegate<fn_09> fn09{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn09)(arg), fn_09_v + arg);
    delegate<fn_10> fn10{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn10)(arg), fn_10_v + arg);
    delegate<fn_11> fn11{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn11)(arg), fn_11_v + arg);
    delegate<fn_12> fn12{ functor_ref_except<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn12)(arg), fn_12_v + arg);
    delegate<fn_13> fn13{ functor_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn13(arg), fn_13_v + arg);
    delegate<fn_14> fn14{ functor_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn14(arg), fn_14_v + arg);
    delegate<fn_15> fn15{ functor_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn15(arg), fn_15_v + arg);
    delegate<fn_16> fn16{ functor_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn16(arg), fn_16_v + arg);
    delegate<fn_17> fn17{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn17(arg), fn_17_v + arg);
    delegate<fn_18> fn18{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn18(arg), fn_18_v + arg);
    delegate<fn_19> fn19{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn19(arg), fn_19_v + arg);
    delegate<fn_20> fn20{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(fn20(arg), fn_20_v + arg);
    delegate<fn_21> fn21{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn21)(arg), fn_21_v + arg);
    delegate<fn_22> fn22{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn22)(arg), fn_22_v + arg);
    delegate<fn_23> fn23{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn23)(arg), fn_23_v + arg);
    delegate<fn_24> fn24{ functor_ref_noexcept<small_t>{ 10 } };
    EXPECT_EQ(std::move(fn24)(arg), fn_24_v + arg);
}

TEST(DelegateTest, CallLargeFunctor)
{
    int arg = 10;

    delegate<fn_01> fn01{ functor_except<large_t>{ 10 } };
    EXPECT_EQ(fn01(arg), fn_01_v + arg);
    delegate<fn_02> fn02{ functor_except<large_t>{ 10 } };
    EXPECT_EQ(fn02(arg), fn_02_v + arg);
    delegate<fn_03> fn03{ functor_except<large_t>{ 10 } };
    EXPECT_EQ(fn03(arg), fn_03_v + arg);
    delegate<fn_04> fn04{ functor_except<large_t>{ 10 } };
    EXPECT_EQ(fn04(arg), fn_04_v + arg);
    delegate<fn_05> fn05{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(fn05(arg), fn_05_v + arg);
    delegate<fn_06> fn06{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(fn06(arg), fn_06_v + arg);
    delegate<fn_07> fn07{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(fn07(arg), fn_07_v + arg);
    delegate<fn_08> fn08{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(fn08(arg), fn_08_v + arg);
    delegate<fn_09> fn09{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn09)(arg), fn_09_v + arg);
    delegate<fn_10> fn10{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn10)(arg), fn_10_v + arg);
    delegate<fn_11> fn11{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn11)(arg), fn_11_v + arg);
    delegate<fn_12> fn12{ functor_ref_except<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn12)(arg), fn_12_v + arg);
    delegate<fn_13> fn13{ functor_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn13(arg), fn_13_v + arg);
    delegate<fn_14> fn14{ functor_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn14(arg), fn_14_v + arg);
    delegate<fn_15> fn15{ functor_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn15(arg), fn_15_v + arg);
    delegate<fn_16> fn16{ functor_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn16(arg), fn_16_v + arg);
    delegate<fn_17> fn17{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn17(arg), fn_17_v + arg);
    delegate<fn_18> fn18{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn18(arg), fn_18_v + arg);
    delegate<fn_19> fn19{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn19(arg), fn_19_v + arg);
    delegate<fn_20> fn20{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(fn20(arg), fn_20_v + arg);
    delegate<fn_21> fn21{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn21)(arg), fn_21_v + arg);
    delegate<fn_22> fn22{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn22)(arg), fn_22_v + arg);
    delegate<fn_23> fn23{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn23)(arg), fn_23_v + arg);
    delegate<fn_24> fn24{ functor_ref_noexcept<large_t>{ 10 } };
    EXPECT_EQ(std::move(fn24)(arg), fn_24_v + arg);
}

TEST(DelegateTest, Lambda)
{
    delegate<int(int)> fn0;

    double ballast = 2.0; // Prevent lambda from turning into function
    auto lambda = [ballast](int arg)->int { return arg; };
    auto lambda_mut = [ballast](int arg)mutable ->int { return arg; };

    // Construction (lambda)
    delegate<int(int)> fn1{ lambda };
    EXPECT_TRUE(fn1);
    EXPECT_NE(fn1, nullptr);
    EXPECT_NE(nullptr, fn1);
    EXPECT_NE(fn1, fn0);
    EXPECT_NE(fn0, fn1);
    EXPECT_EQ(fn1(10), 10);

    // Construction (mutable lambda)
    delegate<int(int)> fn2{ lambda_mut };
    EXPECT_TRUE(fn2);
    EXPECT_NE(fn2, nullptr);
    EXPECT_NE(nullptr, fn2);
    EXPECT_NE(fn2, fn0);
    EXPECT_NE(fn0, fn2);
    EXPECT_EQ(fn2(10), 10);

    // Assignment (lambda)
    delegate<int(int)> fn3;
    fn3 = lambda;
    EXPECT_TRUE(fn3);
    EXPECT_NE(fn3, nullptr);
    EXPECT_NE(nullptr, fn3);
    EXPECT_NE(fn3, fn0);
    EXPECT_NE(fn0, fn3);
    EXPECT_EQ(fn3(10), 10);

    // Assignment (mutable lambda)
    delegate<int(int)> fn4;
    fn4 = lambda_mut;
    EXPECT_TRUE(fn4);
    EXPECT_NE(fn4, nullptr);
    EXPECT_NE(nullptr, fn4);
    EXPECT_NE(fn4, fn0);
    EXPECT_NE(fn0, fn4);
    EXPECT_EQ(fn4(10), 10);

    // Comparison
    delegate<int(int)> fn5{ lambda };
    delegate<int(int)> fn6{ lambda };
    EXPECT_NE(fn5, fn6);
    EXPECT_NE(fn6, fn5);
}

TEST(DelegateTest, MoveOnlyLambda)
{
    auto data = std::make_unique<int>(15);
    auto lambda = [d = std::move(data)](int arg) { return arg + *d; };
    static_assert(!std::is_copy_constructible_v<decltype(lambda)>);
    static_assert(!std::is_copy_assignable_v<decltype(lambda)>);

    delegate<int(int)> fn1{ std::move(lambda) };
    EXPECT_TRUE(fn1);
    EXPECT_EQ(fn1(5), 20);

    delegate<int(int)> fn2{ std::move(fn1) };
    EXPECT_FALSE(fn1);
    EXPECT_TRUE(fn2);
    EXPECT_EQ(fn2(10), 25);
}

TEST(DelegateTest, NoCopyMoveRefs)
{
    int arg = 10;
    functor_no_copy_move functor;
    delegate<int(int)> fn1{ std::ref(functor) };
    EXPECT_TRUE(fn1);
    EXPECT_EQ(fn1(arg), fn_01_v + arg);
    delegate<int(int)const> fn2{ std::cref(functor) };
    EXPECT_TRUE(fn2);
    EXPECT_EQ(fn2(arg), fn_02_v + arg);
}

TEST(DelegateTest, Bind)
{
    using std::placeholders::_1;

    int arg = 10;
    test_class ts;

    delegate<int(int)> fn1{ std::bind(&test_class::method, ts, _1) };
    EXPECT_EQ(fn1(arg), fn_01_v + arg);
}

TEST(DelegateTest, MemFn)
{
    int arg = 10;
    test_class ts;

    delegate<int(test_class, int)> fn1{ std::mem_fn(&test_class::method) };
    EXPECT_EQ(fn1(ts, arg), fn_01_v + arg);
}

TEST(DelegateTest, ThrowOnAssignment)
{
    int arg = 10;
    functor_throw_on_move functor;

    // Small functor
    delegate<int(int)> fn1{ functor_except<small_t>{ 10 } };

    try
    {
        fn1 = functor;
    }
    catch (...)
    {
    }

    // If assignment throws the delegate stays unaffected
    EXPECT_TRUE(fn1);
    EXPECT_EQ(fn1, delegate<int(int)>{ functor_except<small_t>{ 10 } });
    EXPECT_EQ(fn1(arg), fn_01_v + arg);

    // Large functor
    delegate<int(int)> fn2{ functor_except<large_t>{ 10 } };

    try
    {
        fn2 = functor;
    }
    catch (...)
    {
    }

    // If assignment throws the delegate stays unaffected
    EXPECT_TRUE(fn2);
    EXPECT_EQ(fn2, delegate<int(int)>{ functor_except<large_t>{ 10 } });
    EXPECT_EQ(fn2(arg), fn_01_v + arg);
}

TEST(DelegateTest, ThrowOnCall)
{
    functor_throw_on_call functor{ 11 };

    delegate<int(int)> fn1{ functor };

    try
    {
        fn1(10);
    }
    catch (...)
    {
    }

    // If invocation throws the delegate stays unaffected
    EXPECT_TRUE(fn1);
    EXPECT_EQ(fn1, delegate<int(int)>{ functor_throw_on_call{ 11 } });
}

TEST(DelegateTest, FunctionOverload)
{
    delegate<std::string(int)> fn1{ function_overload };
    EXPECT_EQ(fn1(10), std::string{ "10" });
    delegate<int(int, int)> fn2{ function_overload };
    EXPECT_EQ(fn2(3, 4), 7);
}

TEST(DelegateTest, CallVoidNoArgs)
{
    int result = 0;
    auto lambda = [&result]()->void { result = 10; };
    delegate<void()> fn1{ lambda };
    fn1();
    EXPECT_EQ(result, 10);
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}