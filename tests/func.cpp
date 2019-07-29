#include "catch2/catch.hh"

#include <tlab/func.hpp>

TEST_CASE("func"){
    SECTION("0"){
        std::cout << "SECTION 0" << std::endl;
        tlab::func<void()> func;
        REQUIRE_FALSE(func);
    };
    
    SECTION("1"){
        std::cout << "SECTION 1" << std::endl;
        tlab::func<int ()> func([](void)->int {
            return 1;
        });
        REQUIRE(func() == 1);

        REQUIRE(func);
    };

    SECTION("2"){
        std::cout << "SECTION 2" << std::endl;
        tlab::func<int (int)> add_one([](int i)->int {
            return i+1;
        });
        REQUIRE(add_one(2) == 3);
        REQUIRE(add_one(3) == 4);
    }

    SECTION("3"){
        std::cout << "SECTION 3" << std::endl;
        tlab::func<void()> func;
        func = []{};
        func();
    }

    SECTION("4"){
        std::cout << "SECTION 4" << std::endl;
        tlab::func<void()> func([]{});
        tlab::func<void()> func_assign(func);
        REQUIRE(func);
        tlab::func<void()> func_move(std::move(func));
        REQUIRE_FALSE(func);
    }    
}