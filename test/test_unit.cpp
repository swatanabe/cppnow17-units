// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/units2/unit.hpp>
#include <boost/units2/def.hpp>
#include <boost/type_index.hpp>

#define BOOST_TEST_MODULE test_unit
#include <boost/test/unit_test.hpp>

BOOST_UNITS2_DEF(length);
BOOST_UNITS2_DEF(meter, length);
BOOST_UNITS2_DEF(yard, meter * std::ratio<9144,10000>());
BOOST_UNITS2_DEF(foot, yard * std::ratio<1,3>());
BOOST_UNITS2_DEF(inch, foot * std::ratio<1,12>());
BOOST_UNITS2_DEF(xmeter, meter);
BOOST_UNITS2_DEF(sq_meter, meter * meter);

inline constexpr auto centimeter = std::centi() * meter;

struct degree_factor : boost::units2::scale_base {
    static constexpr double value() { return 180/3.14159265358979323846; }
};
BOOST_UNITS2_DEF(angle);
BOOST_UNITS2_DEF(radian, angle);
BOOST_UNITS2_DEF(degree, degree_factor()*radian);

#define TEST_SAME_TYPE(T, U) BOOST_TEST(::boost::typeindex::type_id<decltype(T)>() == ::boost::typeindex::type_id<decltype(U)>())
#define TEST_NOT_SAME_TYPE(T, U) BOOST_TEST(::boost::typeindex::type_id<decltype(T)>() != ::boost::typeindex::type_id<decltype(U)>())

BOOST_AUTO_TEST_CASE(test_multiply)
{
    // Multiplication should yield the same type regardless of argument order.
    TEST_SAME_TYPE(meter * yard, yard * meter);

    // sq_meter is a distinct type from meter*meter.
    TEST_NOT_SAME_TYPE(meter * meter * meter, sq_meter * meter);

    // Multiplication by a std::ratio is defined
    TEST_SAME_TYPE(meter * std::centi(), centimeter);
    TEST_SAME_TYPE(std::centi() * meter, centimeter);

    // Scale factors should collapse...
    TEST_SAME_TYPE(std::deci() * centimeter, std::milli() * meter);
    TEST_SAME_TYPE(centimeter * std::deci(), std::milli() * meter);
    // ...and should be removed entirely when they cancel
    TEST_SAME_TYPE(std::hecto() * centimeter, meter);
    TEST_SAME_TYPE(centimeter * std::hecto(), meter);

    // The scale factor should be reduced to its lowest terms
    TEST_SAME_TYPE((std::ratio<4,2>() * meter), (meter * std::ratio<6,3>()));

    // A scale factor of 1 is ignored
    TEST_SAME_TYPE((std::ratio<1,1>() * meter), meter);
    TEST_SAME_TYPE((meter * std::ratio<1,1>()), meter);
    // ...even if the ratio is only equivalent to 1.
    TEST_SAME_TYPE((std::ratio<3,3>() * meter), meter);
    TEST_SAME_TYPE((meter * std::ratio<3,3>()), meter);
    // ...and it is not ambiguous with folding scale factors.
    TEST_SAME_TYPE((std::ratio<3,3>() * centimeter), centimeter);
    TEST_SAME_TYPE((centimeter * std::ratio<3,3>()), centimeter);
}

// Everything should be calculated using exact arithmetic up to the
// final division.  Therfore, the maximum possible difference is 1 ulp.
BOOST_AUTO_TEST_CASE(test_basic_conversion, * boost::unit_test::tolerance(std::numeric_limits<double>::epsilon()))
{
    // converting a unit to itself always yields a factor of 1.
    BOOST_TEST(conversion_factor(meter, meter) == 1.0);
    BOOST_TEST(conversion_factor(centimeter, centimeter) == 1.0);
    BOOST_TEST(conversion_factor(inch, inch) == 1.0);

    // Normalizing the dimensions should work when a base unit
    // directly uses another base unit.
    BOOST_TEST(conversion_factor(xmeter, meter) == 1.0);

    // conversions should work in both directions
    BOOST_TEST(conversion_factor(inch, centimeter) == 2.54);
    BOOST_TEST(conversion_factor(centimeter, inch) == 100./254);

    // Composite conversions should work.
    BOOST_TEST(conversion_factor(meter * meter, inch * foot), 32.80839895013123358);

    // The result should be correct even when it is too large or too small to
    // be calculated using std::ratio.
    auto nm = std::nano() * meter;
    BOOST_TEST(conversion_factor(nm*nm*nm, meter*meter*meter) == 1e-27);
    BOOST_TEST(conversion_factor(meter*meter*meter, nm*nm*nm) == 1e+27);
}
