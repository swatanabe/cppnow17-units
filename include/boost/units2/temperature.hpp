// Copyright (c) 2017-2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_TEMPERATURE_HPP_INCLUDED
#define BOOST_UNITS2_TEMPERATURE_HPP_INCLUDED

#include <boost/units2/absolute.hpp>
#include <boost/units2/si.hpp>

namespace boost {
namespace units2 {
namespace temperature {

constexpr auto kelvin = absolute_unit(si::kelvin);
constexpr auto celsius = absolute_unit(kelvin, std::ratio<27315,100>());
constexpr auto fahrenheit = absolute_unit(celsius * std::ratio<5,9>(), std::ratio<-32>());

}
}
}

#endif
