#include "catch2/catch.hh"

#include <tlab/member_ptr.hpp>
#include <tlab/member_func_ptr.hpp>

namespace {
    struct member_ptr_test{
        int v0;
        int v1;
        double v2;
        std::string v3;

        int v0value(){
            return v0;
        }
    };
}

TEST_CASE("member ptr & member fn ptr"){
    tlab::member_ptr<member_ptr_test,int> v0_ptr(&member_ptr_test::v0);
    tlab::member_func_ptr<member_ptr_test , int ()> v0_fn_ptr(&member_ptr_test::v0value);
    member_ptr_test test_obj;
    v0_ptr.set(&test_obj,10);
    REQUIRE(v0_ptr.get(test_obj) == 10);
    REQUIRE(v0_fn_ptr(&test_obj) == 10);
    REQUIRE(v0_fn_ptr(test_obj) == 10);
}

