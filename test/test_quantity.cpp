// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/units2/quantity.hpp>
#include <boost/units2/unit.hpp>
#include <boost/units2/def.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_UNITS2_DEF(length);
BOOST_UNITS2_DEF(meter, length);

using boost::units2::quantity;

BOOST_AUTO_TEST_CASE(test_quantity)
{
    quantity<meter> x{1.0};
}
