/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#include <iostream>
#include <memory>

#include <delegate.h>

using vdk::delegate;

namespace
{
struct functor
{
    explicit functor(int data) noexcept
        : data_{ data }
    {}
    void operator()(int arg)
    {
        std::cout << "functor(" << arg << ")" << std::endl;
    }
    void operator()(int arg) const
    {
        std::cout << "functor(" << arg << ")const" << std::endl;
    }
    void operator()(int arg) volatile
    {
        std::cout << "functor(" << arg << ")volatile" << std::endl;
    }
    void operator()(int arg) const volatile
    {
        std::cout << "functor(" << arg << ")const volatile" << std::endl;
    }
    bool operator==(const functor & other) const noexcept
    {
        return data_ == other.data_;
    }
    int data_;
};

struct functor_noexcept
{
    functor_noexcept() noexcept = default;
    void operator()(int arg) & noexcept
    {
        std::cout << "functor_noexcept(" << arg << ")& noexcept" << std::endl;
    }
    void operator()(int arg) && noexcept
    {
        std::cout << "functor_noexcept(" << arg << ")&& noexcept" << std::endl;
    }
};

void function(int arg)
{
    std::cout << "function(" << arg << ")" << std::endl;
}

} // namespace

int main()
{
    // Create an empty delegate
    delegate<void(int)> fn1;

    // Create delegate with a function
    delegate<void(int)> fn2{ function };

    // Create delegate with a function object
    delegate<void(int)> fn3{ functor{ 10 } };

    // Create delegate with a lambda
    double ballast = 2.0;
    // ballast is needed to prevent lambda conversion to function pointer!
    delegate<void(int)> fn4{[ballast](int arg)
    {
        std::cout << "lambda(" << arg << ")" << std::endl;
    }};

    // Check whether delegates are empty
    if (fn1 == nullptr) std::cout << "fn1 is empty" << std::endl;
    if (fn2 == nullptr) std::cout << "fn2 is empty" << std::endl;
    if (fn3 == nullptr) std::cout << "fn3 is empty" << std::endl;
    if (fn4 == nullptr) std::cout << "fn4 is empty" << std::endl;

    // Call only those delegates that are not empty
    if (fn1) fn1(1);
    if (fn2) fn2(2);
    if (fn3) fn3(3);
    if (fn4) fn4(4);

    // Reassign targets for delegates
    fn1 = function;
    fn2 = functor{ 15 };
    fn3 = [ballast](int arg){ std::cout << "lambda(" << arg << ")" << std::endl; };
    
    // Make a delegate empty
    fn4 = nullptr;

    // Compare delegates

    // Both delegates point to the same function
    fn4 = function;
    if (fn1 == fn4) std::cout << "target is function: fn1 == fn4" << std::endl;

    // Compare delegates with comparable and equal targets
    fn4 = functor{ 15 };
    if (fn2 == fn4) std::cout << "target is functor: fn2 == fn4" << std::endl;

    // Compare delegates with comparable but not equal targets
    fn4 = functor{ 20 };
    if (fn2 != fn4) std::cout << "target is functor: fn2 != fn4" << std::endl;

    // Compare delegates with lambdas (!lambdas cannot be compared!)
    auto lambda = [ballast](int arg) { std::cout << "lambda(" << arg << ")" << std::endl; };
    fn3 = lambda;
    fn4 = lambda;
    if (fn3 != fn4) std::cout << "target is lambda: fn3 != fn4" << std::endl;

    // Assign target that cannot be copied
    auto unique = std::make_unique<int>(15);
    auto unique_lambda = [u = std::move(unique)](int arg)
    {
        std::cout << "I am unique lambda with arg = " << arg
                  << " and unique part = " << *u << std::endl;
    };

    fn4 = std::move(unique_lambda);

    // Call stored unique lambda
    fn4(10);

    // Move one delegate into another
    fn1 = std::move(fn4);

    // Check whether delegates are empty and call those that are not
    if (fn1) fn1(15);
    if (fn4) fn4(20); //!note! fn4 is now empty

    // Create delegates with different signatures (some examples)
    delegate<void(int)>                 fn5{ functor{ 10 } };
    delegate<void(int)const>            fn6{ functor{ 10 } };
    delegate<void(int)volatile>         fn7{ functor{ 10 } };
    delegate<void(int)const volatile>   fn8{ functor{ 10 } };
    delegate<void(int)&noexcept>        fn9{ functor_noexcept{} };
    delegate<void(int)&&noexcept>       fn10{ functor_noexcept{} };

    // Call them all
    fn5(5);
    fn6(6);
    fn7(7);
    fn8(8);
    fn9(9);
    std::move(fn10)(10);

    return 0;
}