// Copyright (c) 2017-2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_UNIT_HPP_INCLUDED
#define BOOST_UNITS2_UNIT_HPP_INCLUDED

#include <boost/units2/detail/merge.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/utility.hpp>
#include <boost/integer/common_factor_ct.hpp>
#include <type_traits>
#include <limits>
#include <cstdint>
#include <cmath>

// Design goals:
// - Can represent any unit.
// - Can convert between any units with the same dimensions.
// - New units can be defined in terms of any existing units.
//   Which unit is the base unit and which unit is the derived
//   unit shall not change the behavior of any user code outside
//   the definition itself.
// - The set of dimensions shall be extensible.
// - Every unit shall be represented by one and only one type.
// - To the maximum extent possible, conversions shall not
//   lose precision.
// - The raw type of a unit should be (somewhat) intelligible.
//
// Implementation Notes:
// - All units are ultimately seen as a scaled combination of base units.
// - There is exactly one base unit for each dimension.
// - Conversions reduce both sides to the base form.
// - To avoid loss-of-precision, all conversion factors
//   are tracked and identical components are combined/canceled
//   before any evaluation is done.  In addition, all calculations
//   are carried out using exact (rational) arithmetic wherever possible.
// - The struct that a user defines is a unit, with no wrapping required.
// - All units are reduced to normalized form after every operation.
// - The different types of units can be processed using a visitor via the
//   alias _boost_units2_apply.
//
// Possible improvements:
// - Allow floating point scaling to have greater precision than double.
//   This also means passing the value_type through all the conversion logic.

namespace boost {
namespace units2 {

/*
 * Base class for scales
 */
struct scale_base {
    /// INTERNAL ONLY
    using _boost_units2_is_scale = void;
};

/**
 * Base class for all units.
 */
template<class Derived>
struct unit_base {
    /// INTERNAL ONLY
    /// Tag for use with SFINAE
    using _boost_units2_is_unit = void;
    /// INTERNAL ONLY
    /// visitor to help code that needs to switch on the kind of unit
    template<class F, class T>
    using _boost_units2_apply = typename F::template apply_base<T>;
    /// INTERNAL ONLY
    using _boost_units2_type = Derived;
    /// INTERNAL ONLY
    auto operator<=>(const unit_base&) const = default;
};

/**
 * Represents a unit that is a scaled version of another unit.
 * \pre Base is a Unit
 * \pre Scale is either a std::ratio or a type with a nested static constexpr double value();
 */
template<class Base, class Scale>
struct scaled_unit : unit_base<scaled_unit<Base, Scale> > {
    /// INTERNAL ONLY
    template<class F, class T>
    using _boost_units2_apply = typename F::template apply_scaled<Base, Scale>;
    /// INTERNAL ONLY
    auto operator<=>(const scaled_unit&) const = default;
};

/**
 * Represents a unit that is a combination of other units.
 * \pre All the elements of P must be specializations of dim
 */
template<class... P>
struct compound_unit : unit_base<compound_unit<P...>> {
    /// INTERNAL ONLY
    auto operator<=>(const compound_unit&) const = default;
};

/// INTERNAL ONLY
/// Specializations to make compound_unit useable with merge.
template<>
struct compound_unit<> : unit_base<compound_unit<>> {
    static constexpr const bool empty = true;
    template<class F, class T>
    using _boost_units2_apply = typename F::template apply_compound<>;
    /// INTERNAL ONLY
    auto operator<=>(const compound_unit&) const = default;
};
/// INTERNAL ONLY
template<class P0, class... P>
struct compound_unit<P0, P...> : unit_base<compound_unit<P0,P...>> {
    static constexpr const bool empty = false;
    using front = P0;
    using pop_front = compound_unit<P...>;
    template<class F, class T>
    using _boost_units2_apply = typename F::template apply_compound<P0, P...>;
    /// INTERNAL ONLY
    auto operator<=>(const compound_unit&) const = default;
};

namespace detail {

template<class F, class T>
using visit = typename T::template _boost_units_apply2<F, T>;

constexpr int const_strcmp(const char * lhs, const char * rhs)
{
    return (*lhs && *rhs) ?
        (*lhs == *rhs ? const_strcmp(lhs+1, rhs+1) : (*lhs < *rhs ? -1 : 1)) :
        ((!*lhs && !*rhs) ? 0 : (!*lhs ? -1 : 1));
}

template<class T, class U>
struct scale_compare {
    static constexpr const int value = T::value() < U::value?-1:(T::value()>U::value()?1:0);
};
template<class T, long long N, long long D>
struct scale_compare<T,std::ratio<N,D>>
{
    static const constexpr int value = 1;
};
template<class T, long long N, long long D>
struct scale_compare<std::ratio<N,D>,T>
{
    static const constexpr int value = -1;
};
template<long long N1, long long D1, long long N2, long long D2>
struct scale_compare<std::ratio<N1,D1>,std::ratio<N2,D2>>
{
    static const constexpr int value = std::ratio_less<std::ratio<N1,D1>,std::ratio<N2,D2>>::value?-1:
        (std::ratio_less<std::ratio<N2,D2>,std::ratio<N1,D1>>::value?1:0);
};

// For two user-defined units, compare by name
template<class T, class U>
struct unit_compare_impl {
    static constexpr const int value = const_strcmp(T::name, U::name);
    static_assert(std::is_same<T, U>::value || value != 0, "Different units cannot have the same name.");
};

// Compare compond units lexicographically
template<class T0, class... T, class U0, class... U>
struct unit_compare_impl<compound_unit<T0, T...>, compound_unit<U0, U...> >
{
    static constexpr const int value = unit_compare_impl<typename T0::base, typename U0::base>::value?
        unit_compare_impl<typename T0::base, typename U0::base>::value:
        (std::ratio_less<typename T0::exponent, typename U0::exponent>::value?-1:1);
};
template<class T0, class... T, class... U>
struct unit_compare_impl<compound_unit<T0, T...>, compound_unit<T0, U...> >
{
    static constexpr const int value = unit_compare_impl<compound_unit<T...>, compound_unit<U...>>::value;
};
template<class... T>
struct unit_compare_impl<compound_unit<T...>, compound_unit<> >
{
    static constexpr const int value = 1;
};
template<class... T>
struct unit_compare_impl<compound_unit<>, compound_unit<T...> >
{
    static constexpr const int value = -1;
};
template<>
struct unit_compare_impl<compound_unit<>, compound_unit<> >
{
    static constexpr const int value = 0;
};

// For scaled_unit compare the base first, then the exponent
template<class B1, class E1, class B2, class E2>
struct unit_compare_impl<scaled_unit<B1, E1>, scaled_unit<B2, E2> >
{
    static const constexpr int value = (unit_compare_impl<B1, B2>::value != 0)?
        (unit_compare_impl<B1, B2>::value != 0):
        scale_compare<E1, E2>::value;
};

// cross type comparisons
// user-defined unit < scaled_unit < compound_unit
template<class... T, class U>
struct unit_compare_impl<compound_unit<T...>, U>
{
    static const constexpr int value = 1;
};
template<class T, class... U>
struct unit_compare_impl<T, compound_unit<U...>>
{
    static const constexpr int value = -1;
};
template<class... T, class B, class E>
struct unit_compare_impl<compound_unit<T...>, scaled_unit<B,E> >
{
    static const constexpr int value = 1;
};
template<class B, class E, class... U>
struct unit_compare_impl<scaled_unit<B, E>, compound_unit<U...>>
{
    static const constexpr int value = -1;
};
template<class T, class B, class E>
struct unit_compare_impl<T, scaled_unit<B,E> >
{
    static const constexpr int value = -1;
};
template<class B, class E, class U>
struct unit_compare_impl<scaled_unit<B, E>, U>
{
    static const constexpr int value = 1;
};

// pre: T and U are both compound_units
template<class T, class U>
using compound_unit_multiply = detail::merge<unit_compare_impl, compound_unit, T, U>;

// Wrap any class in a compound_unit iff it is not already a compound_unit.
template<class T>
struct as_compound_unit_impl { typedef compound_unit<dim<T, std::ratio<1, 1> > > type; };
template<class... T>
struct as_compound_unit_impl<compound_unit<T...> > { typedef compound_unit<T...> type; };
template<class T>
using as_compound_unit = typename as_compound_unit_impl<T>::type;

// Unwrap any compound units of the form U^1.
// Folds scaled_units that use std::ratio.
// May perform other normalization as needed in the future.
// Note: It is assumed that this normalization is applied
// consistently, so we never need to fix more than the outer layer.
template<class T>
struct simplify_unit_impl { using type = T; };
template<class T>
struct simplify_unit_impl<compound_unit<dim<T, std::ratio<1, 1> > > > { using type = T; };
template<class T, std::intmax_t N1, std::intmax_t D1, std::intmax_t N2, std::intmax_t D2>
struct simplify_unit_impl<scaled_unit<scaled_unit<T, std::ratio<N1,D1>>,std::ratio<N2,D2>>> {
    using new_scale = std::ratio_multiply<std::ratio<N1,D1>,std::ratio<N2,D2>>;
    using type = boost::mp11::mp_if_c<(new_scale::num==1&&new_scale::den==1), T, scaled_unit<T, new_scale> >;
};
template<class T>
struct simplify_unit_impl<scaled_unit<T,std::ratio<1,1>>> { using type = T; };
// resolve ambiguity
template<class T, std::intmax_t N, std::intmax_t D>
struct simplify_unit_impl<scaled_unit<scaled_unit<T,std::ratio<N,D>>,std::ratio<1,1>>> {
    using type = scaled_unit<T,std::ratio<N,D>>;
};
template<class T>
using simplify_unit = typename simplify_unit_impl<T>::type;

// The result of multiplying two units
template<class T, class U>
using unit_multiply = simplify_unit<compound_unit_multiply<as_compound_unit<T>, as_compound_unit<U> > >;

template<class T, class E>
struct unit_pow_impl;
template<class... T, class... E, class R>
struct unit_pow_impl<compound_unit<dim<T, E>...>, R> {
    using type = compound_unit<dim<T, std::ratio_multiply<E, R> >...>;
};
template<class... T, class... E>
struct unit_pow_impl<compound_unit<dim<T, E>...>, std::ratio<0>> {
    using type = compound_unit<>;
};
template<class T, class E>
using unit_pow = simplify_unit<typename unit_pow_impl<as_compound_unit<T>, E>::type>;

template<class T, class U>
using unit_divide = unit_multiply<T, unit_pow<U, std::ratio<-1>>>;

// determine whether a type is a unit
template<class T>
using requires_unit = typename T::_boost_units2_is_unit;

template<class T, class E = void>
struct is_unit_impl {
    using type = std::false_type;
};
template<class T>
struct is_unit_impl<T, requires_unit<T>> {
    using type = std::true_type;
};

// A scale must either be a std::ratio or inherit from scale_base
// This only handles the latter.
template<class T>
using requires_scale = typename T::_boost_units2_is_scale;

} // namespace detail

template<class T, class U, class = detail::requires_unit<T>, class = detail::requires_unit<U> >
constexpr auto operator*(T, U) -> detail::unit_multiply<T, U>
{ return {}; }

template<class T, class U, class = detail::requires_unit<T>, class = detail::requires_unit<U> >
constexpr auto operator/(T, U) -> detail::unit_divide<T, U>
{ return {}; }

// multiplying a unit by a std::ratio creates a scaled_unit
template<class T, long long N, long long D, class = detail::requires_unit<T>>
constexpr auto operator*(T, std::ratio<N,D>) -> detail::simplify_unit<scaled_unit<T, typename std::ratio<N,D>::type>>
{ return {}; }
template<class T, long long N, long long D, class = detail::requires_unit<T>>
constexpr auto operator*(std::ratio<N,D>, T) -> detail::simplify_unit<scaled_unit<T, typename std::ratio<N,D>::type>>
{ return {}; }

// multiplying a unit by any scale gives a scaled unit
template<class T, class U, class = detail::requires_unit<T>, class = detail::requires_scale<U> >
constexpr auto operator*(T, U) -> detail::simplify_unit<scaled_unit<T, U>>
{ return {}; }
template<class T, class U, class = detail::requires_scale<T>, class = detail::requires_unit<U> >
constexpr auto operator*(T, U) -> detail::simplify_unit<scaled_unit<U,T>>
{ return {}; }

template<std::intmax_t N, class T, class = detail::requires_unit<T>>
constexpr auto pow(T) -> detail::unit_pow<T, std::ratio<N>>
{ return {}; }

    template<class T, std::intmax_t N, std::intmax_t D, class = detail::requires_unit<T>>
constexpr auto pow(T, std::ratio<N,D>) -> detail::unit_pow<T, std::ratio<N,D>>
{ return {}; }

// Conversion support

namespace detail {

template<class... D>
struct scale_list;

template<>
struct scale_list<> {
    static const constexpr bool empty = true;
};

template<class D0, class... D>
struct scale_list<D0, D...> {
    static const constexpr bool empty = false;
    using front = D0;
    using pop_front = scale_list<D...>;
};

template<class T, class E>
struct scale_list_pow_impl;
template<class... T, class... E, class R>
struct scale_list_pow_impl<scale_list<dim<T, E>...>, R> {
    using type = scale_list<dim<T, std::ratio_multiply<E, R> >...>;
};
template<class T, class E>
using scale_list_pow = typename scale_list_pow_impl<T, E>::type;

template<class T, class U>
using scale_list_multiply = detail::merge<scale_compare, scale_list, T, U>;

struct flatten_scale_impl;
template<class T>
using flatten_scale = visit<flatten_scale_impl, T>;
struct flatten_scale_impl
{
    template<class T>
    using apply_base = scale_list<>;

    template<class Base, class Scale>
    using apply_scaled = scale_list_multiply<flatten_scale<Base>, scale_list<dim<Scale, std::ratio<1>>>>;

    template<class... T>
    using apply_compound = boost::mp11::mp_fold<boost::mp11::mp_list<scale_list_pow<flatten_scale<typename T::base>, typename T::exponent>...>, scale_list<>, scale_list_multiply>;
};

struct dimension_check_impl;
template<class T>
using dimension_check = visit<dimension_check_impl, T>;
struct dimension_check_impl
{
    template<class T>
    using apply_base = typename T::_boost_units2_type;

    template<class Base, class Scale>
    using apply_scaled = dimension_check<Base>;

    template<class... T>
    using apply_compound = ::boost::mp11::mp_fold< boost::mp11::mp_list<unit_pow<dimension_check<typename T::base>, typename T::exponent>...>, compound_unit<>, unit_multiply>;
};

template<class T>
constexpr double get_value(T) { return T::value(); }
template<long long N, long long D>
constexpr double get_value(std::ratio<N, D>) { return static_cast<double>(N)/D; }

template<class T, class U>
struct multiplier {
    static constexpr double value() { return ::boost::units2::detail::get_value(T()) * ::boost::units2::detail::get_value(U()); }
};

template<class B, class E>
struct power {
    static /*constexpr*/ double value() { return ::std::pow(::boost::units2::detail::get_value(B()), ::boost::units2::detail::get_value(E())); }
};

// Returns 0 if overflow would happen
// precondition: all ratios are positive
template<class T, class U>
struct safe_ratio_multiply
{
    static const constexpr long long gcd1 = ::boost::integer::static_gcd<T::num, U::den>::value;
    static const constexpr long long gcd2 = ::boost::integer::static_gcd<U::num, T::den>::value;
    static const constexpr bool overflow =
        (std::numeric_limits<long long>::max()/(T::num/gcd1) > (U::num/gcd2)) ||
        (std::numeric_limits<long long>::max()/(T::den/gcd1) > (U::den/gcd1));
    using type = std::ratio<
        overflow?0:(T::num/gcd1)*(U::num/gcd2),
        overflow?1:(T::den/gcd1)*(U::den/gcd1)>;
};

constexpr long long safe_multiply(long long lhs, long long rhs)
{
    return (std::numeric_limits<long long>::max)()/lhs >= rhs? lhs*rhs : 0;
}
constexpr long long safe_square(long long arg)
{
    return safe_multiply(arg, arg);
}
constexpr long long safe_power(long long base, long long exponent) {
    return exponent == 1? base : safe_multiply(safe_square(safe_power(base, exponent/2)), (exponent%2?base:1));
}

template<class B, long long E>
struct safe_ratio_pow {
    static const constexpr long long abs_exponent = E < 0? -E : E;
    static const constexpr long long num = safe_power(E<0?B::den:B::num, abs_exponent);
    static const constexpr long long den = safe_power(E<0?B::num:B::den, abs_exponent);
    static const constexpr bool overflow = num==0||den==0;
    using type = std::ratio<overflow?0:num,overflow?1:den>;
};

template<class T, class U>
struct fold_conversion_impl { using type = multiplier<T, U>; };

template<long long N1, long long D1, long long N2, long long D2>
struct fold_conversion_impl<std::ratio<N1,D1>,std::ratio<N2,D2>>
{
    using result1 = typename safe_ratio_multiply<std::ratio<N1, D1>, std::ratio<N2, D2> >::type;
    using type = ::boost::mp11::mp_if_c<result1::num!=0, result1, multiplier<std::ratio<N1,D1>,std::ratio<N2,D2> > >;
};

template<class T>
struct evaluate_power;
template<class Base, class Exponent>
struct evaluate_power<dim<Base, Exponent> >
{
    using type = power<Base, Exponent>;
};
template<long long N, long long D, long long E>
struct evaluate_power<dim<std::ratio<N,D>, std::ratio<E> > >
{
    using result1 = typename safe_ratio_pow<std::ratio<N,D>, E>::type;
    using type = ::boost::mp11::mp_if_c<result1::num!=0, result1, power<std::ratio<N,D>, std::ratio<E> > >;
};

template<class T, class U>
using conversion_fold_op = typename fold_conversion_impl<T,U>::type;

template<class... T>
struct fold_conversion;
template<class... T>
struct fold_conversion<scale_list<T...>> {
    using type = boost::mp11::mp_fold<boost::mp11::mp_list<typename evaluate_power<T>::type...>,std::ratio<1>,conversion_fold_op>;
};

template<class T, class U>
void check_conversion() {
    static_assert(std::is_same<T, U>::value,
        "Cannot convert units with different dimensions.");
}

} // namespace detail

template<class T, class U, class = detail::requires_unit<T>, class = detail::requires_unit<U>>
constexpr bool has_same_dimension(T, U)
{
    return std::is_same<detail::dimension_check<T>, detail::dimension_check<U>>::value;
}

template<class T, class U, class = detail::requires_unit<T>, class = detail::requires_unit<U>>
constexpr double conversion_factor(T, U)
{
    // Indirection to make sure that the reduced dimensions appear
    // in the template backtrace.
    detail::check_conversion<detail::dimension_check<T>, detail::dimension_check<U>>();
    return ::boost::units2::detail::get_value(typename detail::fold_conversion<detail::flatten_scale<detail::unit_divide<T, U>>>::type());
}

}
}

#endif
