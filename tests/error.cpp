#include "catch2/catch.hh"

#include <tlab/error.hpp>

TEST_CASE("error"){
    auto ec = std::make_error_code(std::errc::broken_pipe);
    REQUIRE(ec == tlab::errc::fail);
}

