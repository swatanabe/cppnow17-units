// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_CONVERSION_HPP_INCLUDED
#define BOOST_UNITS2_CONVERSION_HPP_INCLUDED

namespace boost {
namespace units2 {

// some conversions may be allowed conditionally.
// for example, radians is dimensionless.
//
// To handle this, additional arguments may be
// passed to conversion_factor, to indicate
// valid conversions.
//
// Algorithm: Divide the units as for a regular conversion
// then match the extra conversions against the quotient.
//
// extra conversions are reduced to a dimension + scale
//
// We solve for x1, x2, ..., xn in:
// x1 * dim1 + x2 * dim2 + ... + xn * dimn = quotient
// and then multiply (or maybe divide?) by scale1^x1 * scale2^x2 * ... * scale2^x3

template<class... T>
struct conversion_list {};

dimension -> factor;

namespace detail {

struct runtime_ratio { std::intmax_t num, den; };

constexpr array<runtime_ratio, N> solve(...);

inline constexpr void solve(int* matrix, int M, int N, int * vec, int * out)
{
    for(int i = 0; i < M; ++i)
    {
        // find a column with a non-zero initial element and swap it into place
        for(int j = i; j < M; ++j)
        {
            if(matrix[j * N + i] != 0)
            {
                for(int k = i; k < N; ++k)
                {
                    swap(matrix[j * N + k], matrix[i * N + k]);
                }
                swap(vec[j], vec[i]);
            }
        }
    }
}

template<class Unit, class N>

template<class Scale>

}

template<class T, class U, class... Xtra>
double conversion_factor(T, U, Xtra...)
{
    (T/U) = 5;
}

conversion_factor(radians/second, hertz, radians==dimensionless);

}
}

#endif
