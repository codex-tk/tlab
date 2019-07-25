#include "catch2/catch.hh"

#if defined(ENABLE_LIB_BOOST)

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;

TEST_CASE("BoostLog" , "Debug"){
    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info);
    
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
}
#endif
