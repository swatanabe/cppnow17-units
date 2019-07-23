// Copyright (c) 2017-2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_DETAIL_MERGE_HPP_INCLUDED
#define BOOST_UNITS2_DETAIL_MERGE_HPP_INCLUDED

#include <ratio>

// Defines a merge operation for sequences of (Base, Exponent) pairs.
// Identical bases are combined, and 0 exponents are removed.

// The sequences must have the following members:
// empty: bool
// pop_front: sequence
// front: a specialization of dim

namespace boost {
namespace units2 {

template<class B, class E>
struct dim {
    typedef B base;
    typedef E exponent;
};

namespace detail {

// Test is the result of a three-way comparison of the heads of the two sequences.
template<int Test>
struct merge_impl;

// Handles the end of either or both sequences
template<bool HasT, bool HasU>
struct merge_recurse_impl;

template<>
struct merge_recurse_impl<true, true> {
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = typename merge_impl<Cmp<typename T::front::base, typename U::front::base>::value>::template apply<Cmp, L, T, U, R...>;
};

// appends two sequences
template<class T, class U>
struct append;

template<template<class ...> class L, class...T, class...U>
struct append<L<T...>, L<U...> > {
    using type = L<T..., U...>;
};

// base case: one or both sequences are empty

template<>
struct merge_recurse_impl<true, false>
{
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = typename append<L<R...>, T>::type;
};

template<>
struct merge_recurse_impl<false, true>
{
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = typename append<L<R...>, U>::type;
};

template<>
struct merge_recurse_impl<false, false>
{
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = L<R...>;
};

// 

template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
using merge_recurse = typename merge_recurse_impl<!T::empty, !U::empty>::template apply<Cmp, L, T, U, R...>;

// Copy the smaller element to the result

template<>
struct merge_impl<1> {
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = merge_recurse<Cmp, L, T, typename U::pop_front, R..., typename U::front>;
};

template<>
struct merge_impl<-1> {
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = merge_recurse<Cmp, L, typename T::pop_front, U, R..., typename T::front>;
};

// If the elements are equal, then add the exponents, but remove
// it entirely if the resulting exponents is 0.

template<bool ExponentsCancel>
struct merge_combine_impl;

template<>
struct merge_combine_impl<true>
{
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class X, class... R>
    using apply = merge_recurse<Cmp, L, T, U, R...>;
};

template<>
struct merge_combine_impl<false>
{
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class X, class... R>
    using apply = merge_recurse<Cmp, L, T, U, R..., X>;
};

template<>
struct merge_impl<0> {
    template<template<class, class> class Cmp, template<class...> class L, class T, class U, class... R>
    using apply = typename merge_combine_impl<
        std::ratio_add<typename T::front::exponent, typename U::front::exponent>::num == 0
    >::template apply<Cmp, L, typename T::pop_front, typename U::pop_front,
        dim<typename T::front::base,
            std::ratio_add<typename T::front::exponent, typename U::front::exponent> >, R...>;
};

//
template<template<class, class> class Cmp, template<class...> class L, class T, class U>
using merge = merge_recurse<Cmp, L, T, U>;

}
}
}

#endif
