// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_DIMENSIONS_HPP_INCLUDED
#define BOOST_UNITS2_DIMENSIONS_HPP_INCLUDED

#include <boost/units2/def.hpp>
#include <boost/units2/unit.hpp>

namespace boost {
namespace units2 {

BOOST_UNITS2_DEF(length);
BOOST_UNITS2_DEF(mass);
BOOST_UNITS2_DEF(time);
BOOST_UNITS2_DEF(temperature);
BOOST_UNITS2_DEF(amount);
BOOST_UNITS2_DEF(current);
BOOST_UNITS2_DEF(luminous_intensity);
BOOST_UNITS2_DEF(angle);
BOOST_UNITS2_DEF(solid_angle);

inline constexpr const auto velocity = length/time;
inline constexpr const auto acceleration = velocity/time;
inline constexpr const auto force = mass*acceleration;
inline constexpr const auto energy = force*length;

}
}

#endif
