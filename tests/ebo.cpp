#include "catch2/catch.hh"

#include <tlab/ebo_storage.hpp>
TEST_CASE("ebo"){
    struct empty{
        int run(void){ return 81; }
    };
    tlab::ebo_storage<int,empty> ebo{1};
    static_assert(sizeof(tlab::ebo_storage<int,empty>) == sizeof(int));
    REQUIRE(ebo.template get<int>() == 1);
    REQUIRE(ebo.get<empty>().run() == 81);
}