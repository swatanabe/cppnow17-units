// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_DIMENSIONS_HPP_INCLUDED
#define BOOST_UNITS2_DIMENSIONS_HPP_INCLUDED

#include <boost/units2/unit.hpp>
#include <boost/units2/def.hpp>
#include <boost/units2/dimensions.hpp>
#include <ratio>

namespace boost {
namespace units2 {
namespace si {

BOOST_UNITS2_DEF(meter,length);
BOOST_UNITS2_DEF(gram,mass);
BOOST_UNITS2_DEF(second,time);
BOOST_UNITS2_DEF(kelvin,temperature);
BOOST_UNITS2_DEF(mole,amount);
BOOST_UNITS2_DEF(ampere,current);
BOOST_UNITS2_DEF(candela,luminous_intensity);

// Extra units that are techically dimensionless
BOOST_UNITS2_DEF(radian,angle);
BOOST_UNITS2_DEF(steradian,solid_angle);

// kilograms are actually the base unit, but for the sake
// of naming consistency, we're defining it this way, since
// it doesn't change the behavior significantly.
inline constexpr const auto kilogram = std::kilo() * gram;

inline constexpr const auto hertz = pow<-1>(second);
inline constexpr const auto newton = meter*kilogram/pow<2>(second);
inline constexpr const auto pascal = newton/pow<2>(meter);
inline constexpr const auto joule = newton*meter;
inline constexpr const auto watt = joule/second;
inline constexpr const auto couloumb = second * ampere;
inline constexpr const auto volt = watt/ampere;
inline constexpr const auto farad = couloumb/volt;
inline constexpr const auto ohm = volt/ampere;
inline constexpr const auto siemens = ampere/volt;
inline constexpr const auto weber = volt*second;
inline constexpr const auto tesla = weber/pow<2>(meter);
inline constexpr const auto henry = weber/ampere;
// celsius = kelvin + std::ratio<27315,100>
inline constexpr const auto lumen = candela*steradian;
inline constexpr const auto lux = lumen/pow<2>(meter);
inline constexpr const auto becquerel = pow<-1>(second);
inline constexpr const auto gray = joule/kilogram;
inline constexpr const auto sievert = joule/kilogram;
inline constexpr const auto katal = mole/second;

}
}
}

#endif
