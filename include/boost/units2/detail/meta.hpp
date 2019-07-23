// Copyright (c) 2018 Steven Watanabe
//
// Distributed under the Boost Software License Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNITS2_DETAIL_META_HPP_INCLUDED
#define BOOST_UNITS2_DETAIL_META_HPP_INCLUDED

#include <boost/units2/def.hpp>
#include <boost/units2/unit.hpp>

namespace boost {
namespace units2 {
namespace detail {

// std::experimental
template<class T, class Enable, template<class...> class F, class... A>
struct detected_or_impl { using type = T; };
template<class T, template<class...> class F, class... A>
struct detected_or_impl<T, std::void_t<F<A...>>, F, A...>
{
    using type = F<A...>;
};
template<class T, template<class...> class F, class... A>
using detected_or = typename detected_or_impl<T, void, F, A...>::type type;

}
}
}

#endif
