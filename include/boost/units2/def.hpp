// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_DEF_HPP_INCLUDED
#define BOOST_UNITS2_DEF_HPP_INCLUDED

#include <boost/units2/unit.hpp>

namespace boost {
namespace units2 {
namespace detail {

template<class T>
unit_base<T> choose_unit_base(const char* = nullptr);
template<class T, class U, class = requires_unit<U>>
U choose_unit_base(U);
template<class T, class U, class = requires_unit<U>>
U choose_unit_base(const char *, U);

struct default_name {
    constexpr const char * operator()(const char* arg) { return arg; }
};
struct explicit_name {
    const char * value;
    constexpr const char * operator()(const char *) { return value; }
};

inline constexpr default_name choose_unit_name() { return {}; }
inline constexpr explicit_name choose_unit_name(const char * arg) { return { arg }; }
template<class T, class = requires_unit<T>>
inline constexpr default_name choose_unit_name(T) { return {}; }
template<class T, class = requires_unit<T>>
inline constexpr explicit_name choose_unit_name(const char * arg, T) { return { arg }; }

}
}
}

/**
 * Defines a new unit or dimension with a given name.  After this macro
 * is used, @c id will name a constant representing the unit.  The
 * type of the unit is available as id ## _t.
 *
 * Takes an optional second parameter which is a string literal
 * identifying the unit.  If not specified, defaults to #id.  This
 * string must be globally unique.
 *
 * The new unit is defined in terms of another unit, which is passed
 * as the last parameter.  When no other unit is provided, defines
 * a dimension.
 *
 * This macro must be used at namespace scope and must be terminated with
 * a semicolon.
 *
 * Examples:
 * \code
 * // Define length as a new dimension
 * BOOST_UNITS2_DEF(length);
 * // A meter is the canonical unit of length
 * BOOST_UNITS2_DEF(meter, length);
 * // An inch is 2.54 centimeters
 * BOOST_UNITS2_DEF(inch, std::ratio<254,100>() * std::centi() * meter);
 * // Distinguish a metric ton from other tons.
 * BOOST_UNITS2_DEF(ton, "ton (metric)", std::ratio<1000>() * kilogram);
 * \endcode
 * 
 * Note that it is often prefereble to define composite units
 * as simple variables.
 * \code
 * constexpr auto milliimeter = std::milli() * meter;
 * \endcode
 *
 * Units defined by BOOST_UNITS2_DEF are primitive in the sense
 * that the library never x-rays the definition when combining units.
 * In particular, <code>std::kilo() * millimeter</code> is the same type as
 * meter, but std::micro() * ton is not the same as gram.
 */
#define BOOST_UNITS2_DEF(id, ...)                                   \
struct id ## _t :                                                   \
    decltype(::boost::units2::detail::choose_unit_base<id ## _t>(__VA_ARGS__))\
{                                                                   \
    static constexpr const char * name =                            \
    ::boost::units2::detail::choose_unit_name(__VA_ARGS__)(#id);    \
    auto operator<=>(const id ## _t&) const = default;              \
};                                                                  \
inline constexpr const id ## _t id{}

#endif
