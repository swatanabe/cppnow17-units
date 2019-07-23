// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_QUANTITY_HPP_INCLUDED
#define BOOST_UNITS2_QUANTITY_HPP_INCLUDED

#include <boost/units2/unit.hpp>

namespace boost {
namespace units2 {

using dimensionless = compound_unit<>;

namespace detail {

template<class T>
using requires_dimensionless = mp11::mp_if_c<std::is_same<T, dimensionless>::value,void>;

}

template<auto Unit, class T=double>
class quantity {
public:
    /// INTERNAL ONLY
    using _boost_units2_is_quantity = void;
    using unit_type = decltype(Unit);
    constexpr quantity() = default;
    static constexpr quantity from_value(const T& x) { return quantity{x}; }
    static constexpr quantity from_value(T&& x) { return quantity{static_cast<T&&>(x)}; }
    constexpr const T& value() const & { return value_; }
    constexpr T&& value() && { return value_; }
    // Implicit conversion to the value_type is valid for dimensionless quantities
    template<class = detail::requires_dimensionless<decltype(Unit)>>
    constexpr operator const T& () const & { return value_; }
    template<class = detail::requires_dimensionless<decltype(Unit)>>
    constexpr operator T&& () && { return value_; }
    static constexpr auto unit() -> decltype(Unit) { return {}; }
private:
    explicit constexpr quantity(const T& x) : value_(x) {}
    explicit constexpr quantity(T&& x) : value_(static_cast<T&&>(x)) {}
    T value_;
};

namespace detail {
// Helper to simplify construction.
template<class T>
struct from_value {
    T&& value;
    template<auto Unit, class U>
    constexpr operator quantity<Unit, U>() const
    { return quantity<Unit, U>::from_value(static_cast<T&&>(value)); }
};
template<class T>
from_value(T&&) -> from_value<T>;

template<class T>
using requires_numeric = ::boost::mp11::mp_if_c<std::numeric_limits<std::remove_reference_t<T>>::is_specialized, void>;

template<class T>
using requires_quantity = typename std::remove_reference_t<T>::_boost_units2_is_quantity;

struct requires_any_unit_impl
{
    template<class T>
    using apply_base = void;
    template<class T, class Scale>
    using apply_scaled = void;
    template<class...>
    using apply_compound = void;
    template<class Unit, class Offset>
    using apply_absolute = void;
};
template<class T>
using requires_any_unit = visit<requires_any_unit_impl, T>;

}

// +-*/, unary +-
// operator<=>

// Quantity * Quantity
template<class T, class U, class=detail::requires_quantity<T>, class=detail::requires_quantity<U>>
constexpr auto operator*(T&& q1, U&& q2) -> quantity<decltype(q1.unit() * q2.unit()){}, decltype(q1.value() * q2.value())>
{ return detail::from_value{static_cast<T&&>(q1).value() * static_cast<U&&>(q2).value()}; }

// Quantity * value
template<class T, class U, class=detail::requires_quantity<T>, class = detail::requires_numeric<U>>
constexpr auto operator*(T&& q, U&& x) -> quantity<decltype(q.unit()){} * dimensionless{}, decltype(q.value() * x)>
{ return detail::from_value{static_cast<T&&>(q).value() * static_cast<U&&>(x)}; }
template<class T, class U, class=detail::requires_numeric<T>, class=detail::requires_quantity<U>>
constexpr auto operator*(T&& x, U&& q) -> quantity<dimensionless{} * decltype(q.unit()){}, decltype(x * q.value())>
{ return detail::from_value{static_cast<T&&>(x) * static_cast<U&&>(q).value()}; }

// Unit * value
template<class Unit, class T, class=detail::requires_any_unit<Unit>, class=detail::requires_numeric<T>>
constexpr auto operator*(Unit, T&& x) -> quantity<Unit{}, T>
{ return detail::from_value{static_cast<T&&>(x)}; }
template<class Unit, class T, class=detail::requires_any_unit<Unit>, class=detail::requires_numeric<T>>
constexpr auto operator*(T&& x, Unit) -> quantity<Unit{}, T>
{ return detail::from_value{static_cast<T&&>(x)}; }

// Quantity * Unit
template<class Q, class Unit2, class=detail::requires_quantity<Q>, class=detail::requires_unit<Unit2>>
constexpr auto operator*(Q&& q, Unit2) -> quantity<decltype(q.unit()){} * Unit2{}, std::decay_t<decltype(q.value())>>
{ return detail::from_value{static_cast<Q&&>(q).value()}; }
template<class Unit1, class Q, class=detail::requires_unit<Unit1>, class=detail::requires_quantity<Q>>
constexpr auto operator*(Unit1, Q&& q) -> quantity<Unit1{} * decltype(q.unit()){}, std::decay_t<decltype(q.value())>>
{ return detail::from_value{static_cast<Q&&>(q).value()}; }

// Quantity +- Quantity (other combinations are not supported for addition)
template<class Q1, class Q2, class=detail::requires_quantity<Q1>, class=detail::requires_quantity<Q2>>
constexpr auto operator+(Q1&& q1, Q2&& q2) -> quantity<decltype(q1.unit() + q2.unit()){}, decltype(q1.value() + q2.value())>
{ return detail::from_value(static_cast<Q1&&>(q1).value() + static_cast<Q2&&>(q2).value()); }
template<class Q1, class Q2, class=detail::requires_quantity<Q1>, class=detail::requires_quantity<Q2>>
constexpr auto operator-(Q1&& q1, Q2&& q2) -> quantity<decltype(q1.unit() - q2.unit()){}, decltype(q1.value() + q2.value())>
{ return detail::from_value(static_cast<Q1&&>(q1).value() - static_cast<Q2&&>(q2).value()); }

// Comparison operators are permitted for identical units
template<auto Unit, class T1, class T2>
constexpr auto operator<=>(const quantity<Unit, T1>& q1, const quantity<Unit, T2>& q2) -> decltype(q1.value() <=> q2.value())
{ return q1.value() <=> q2.value(); }

}
}

#endif
