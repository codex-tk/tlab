#include "catch2/catch.hh"
#include <sstream>
#include <iostream>
#include <tlab/mp.hpp>
#include <tlab/log.hpp>

struct context{
    std::string tag;
};

template<char C>
struct char2type{};

template<char ... Cs>
using chars = tlab::type_list< char2type<Cs> ... >;

template <typename T>
struct dispatch{
    template < typename Stream , typename Context >
    static void invoke(Stream&& s, Context&& c){
        T::write(std::forward<Stream>(s),std::forward<Context>(c));
    }
};

template <char C>
struct dispatch<char2type<C>>{
    template < typename Stream , typename Context >
    static void invoke(Stream&& s, Context&& ){
        s << C;
    }
};

template <char ... Cs>
struct dispatch<tlab::type_list< char2type<Cs> ... >>{
    template < typename Stream , typename Context >
    static void invoke(Stream&& s, Context&& ){
        ((s << Cs) ,...);
    }
};

template < typename ... Ts >
struct static_formatter{
    template < typename Stream , typename Context >
    static void format(Stream&& s, Context&& c){
        (dispatch<Ts>::invoke(s,c) , ...);
    }
};

template < typename Prefix , typename Surfix, typename ... Ts >
struct wrap_each{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){
        ((  dispatch<Prefix>::invoke(s,c),
            dispatch<Ts>::invoke(s,c),
            dispatch<Surfix>::invoke(s,c)), ...);
    }
};

template <typename ... Ts >
using square_bracket_wrap = wrap_each< char2type<'['>, char2type<']'>, Ts... >;

struct tag {
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){
        s << c.tag;
    }
};

TEST_CASE("Log" , "Concept"){
    std::stringstream ss;

    static_formatter<char2type<'['> , char2type<'t'> , char2type<'t'> , char2type<']'>>::format(ss,nullptr);
    REQUIRE(ss.str() == "[tt]");
    ss.str("");

    static_formatter<chars<'[','t','t',']'>>::format(ss,nullptr);
    REQUIRE(ss.str() == "[tt]");
    ss.str("");

    static_formatter< square_bracket_wrap<
            char2type<'1'>, char2type<'2'>, chars<'3','4','5'> >>::format(ss,nullptr);
    REQUIRE(ss.str() == "[1][2][345]");
    ss.str("");

    static_formatter< wrap_each< chars<'[','{'> , chars<'}',']'> , 
            char2type<'1'>, char2type<'2'>, chars<'3','4','5'> >>::format(ss,nullptr);
    REQUIRE(ss.str() == "[{1}][{2}][{345}]");
    ss.str("");

    static_formatter< square_bracket_wrap<
            char2type<'1'>, tag , chars<'3','4','5'> >>::format(ss,context{"tag"});
    REQUIRE(ss.str() == "[1][tag][345]");
    ss.str("");
}

TEST_CASE("log" , "simple"){
    tlab::log::logger< int , double , std::string > logger;

    logger.service<0>() = 10;
    logger.service<1>() = 1.0;
    logger.service<2>() = "String";

    TLOG(logger,tlab::log::level::debug,"tag","%s msg" , "test");
}