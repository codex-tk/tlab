#include "catch2/catch.hh"
#include <tlab/mp.hpp>

using namespace tlab::internal;

TEST_CASE("MetaProgramming" , "[MP]") {
    static_assert(
        std::is_same<
            index_sequence<0>,
            make_index_sequence<1>::type>::value);
    static_assert(
        std::is_same<
            index_sequence<0,1,2,3,4,5,6,7,8,9,10,11>,
            make_index_sequence<12>::type>::value);
}