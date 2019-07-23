// Copyright (c) 2017-2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_ABSOLUTE_HPP_INCLUDED
#define BOOST_UNITS2_ABSOlUTE_HPP_INCLUDED

#include <boost/units2/unit.hpp>
#include <type_traits>

namespace boost {
namespace units2 {

// celsius - celsius = kelvin
// celsius + kelvin = celsius
// fahrenheit
template<class Unit, class Offset=std::ratio<0>>
struct absolute_unit {
    absolute_unit() = default;
    constexpr absolute_unit(Unit, Offset) {}
    /// INTERNAL ONLY
    template<class F, class T>
    using _boost_units2_apply = typename F::template apply_absolute<Unit, Offset>;
};

template<class Unit, class Offset, class Offset2>
operator+(absolute_unit<U, Offset>, Offset2) -> absolute_unit<U, Offset + Offset2>;

template<class Unit, class Offset, class Scale, class=requires_scale<Scale>>
auto operator*(absolute_unit<U, Offset>, Scale) -> absolute_unit<scaled_unit<absolute_unit<U, Offset>, Factor> >
{ return {}; }
template<class Unit, std::intmax_t N1, std::intmax_t D1, std::intmax_t N2, std::intmax_t D2>
auto operator*(absolute_unit<Unit, std::ratio<N1,D1>>, std::ratio<N2,D2>) -> absolute_unit<decltype(Unit{} * std::ratio<N2, D2>), std::ratio_divides<std::ratio<N1,D1>, std::ratio<N2,D2>>>
{ return {}; }

namespace detail {

struct requires_absolute_impl
{
    template<class Unit, class Offset>
    using apply_absolute = void;
};
template<class T>
using requires_absolute = visit<requires_absolute_impl, T>;

struct unitdiff_visitor;

template<class Unit>
using unitdiff_visit = visit<unitdiff_visitor, T>;

template<class T>
using unitdiff_t = detected_or<T, unitdiff_visit, T>;

struct unitdiff_visitor
{
    template<class Unit, class Scale>
    using apply_scaled = scaled_unit<unitdiff_visit<Unit>, Scale>;
    template<class Unit, class Offset>
    using apply_absolute = unitdiff_t<Unit>;
};

}

template<class Unit, class Offset>
constexpr auto operator-(absolute_unit<Unit, Offset>, abosolute_unit<Unit, Offset>) -> detail::unitdiff_t<Unit>
{ return {}; }

template<class Unit, class Offset>
constexpr auto operator+(absolute_unit<Unit, Offset>, detail::unitdiff_t<Unit>) -> absolute_unit<Unit, Offset>
{ return {}; }

template<class Unit, class Offset>
constexpr auto operator+(detail::unitdiff_t<Unit>, absolute_unit<Unit, Offset>) -> absolute_unit<Unit, Offset>
{ return {}; }

template<class Unit, class Offset>
constexpr auto operator-(absolute_unit<Unit, Offset>, detail::unitdiff_t<Unit>) -> absolute_unit<Unit, Offset>
{ return {}; }

template<class Unit, class Offset>
constexpr auto operator+(absolute_unit<Unit, Offset>) -> absolute_unit<Unit, Offset>
{ return {}; }

namespace detail {

struct offset_base_case
{
    template<class Unit>
    using apply_base = void;
    template<class... T>
    using apply_composite = void;
};

template<class Unit, class=visit<offset_base_case, Unit>>
constexpr auto base_offset(Unit) -> double { return 0; }
template<class Unit, class Offset>
constexpr auto base_offset(absolute_unit<Unit, Offset>) -> double
{ return detail::base_offset(Unit{}) + detail::get_value(Offset{}); }
template<class Unit, class Scale>
constexpr auto base_offset(scaled_unit<Unit, Scale>) -> double
{ return detail::base_offset(Unit{}) * detail::get_value(Scale{}); }

}

template<class From, class To>
auto conversion_offset(From, To)
{
    if constexpr(is_absolute)
}

template<class From, class To, class T, class=requires_absolute<From>, class=requires_absolute<To>>
constexpr auto convert(From, To, T&& value)
{
    return (value - detail::base_offset(From)) * conversion_factor(detail::unitdiff_t<From>{}, detail::unitdiff_t<To>{}) + detail::base_offset(To{});
}

}
}
