#include "catch2/catch.hh"

#include <tlab/container_of.hpp>

namespace {
    struct test_object{
        int v0;
        int v1;
        double v2;
        std::string v3;
    };
}

TEST_CASE("container_of"){
    test_object obj;
    test_object* pobj = container_of( &obj.v2 , &test_object::v2 );
    REQUIRE(&obj == pobj);
}

