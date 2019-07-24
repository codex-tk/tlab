#include "catch2/catch.hh"
#include <sstream>
#include <iostream>
#include <tlab/mp.hpp>

struct context{};

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
struct basic_format{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){
        (dispatch<Ts>::invoke(s,c) , ...);
    }
};

template < typename Prefix , typename Surfix, typename ... Ts >
struct attr_wrap{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){
        ((  dispatch<Prefix>::invoke(s,c),
            dispatch<Ts>::invoke(s,c),
            dispatch<Surfix>::invoke(s,c)), ...);
    }
};

TEST_CASE("Log" , "Concept"){
    basic_format<char2type<'['> , char2type<'t'> , char2type<'t'> , char2type<']'>> f;
    std::stringstream ss;
    f.write(ss,nullptr);
    std::cout<<ss.str()<<std::endl;

    basic_format<chars<'[','t','t',']'>> f2;
    ss.clear();
    f2.write(ss,nullptr);
    std::cout<<ss.str()<<std::endl;

    basic_format< 
        attr_wrap<
            char2type<'['>,
            char2type<']'>, 
            char2type<'1'>, char2type<'2'>, chars<'3','4','5'> >> f3;
    ss.clear();
    f3.write(ss,nullptr);
    std::cout<<ss.str()<<std::endl;

    basic_format< 
        attr_wrap<
            chars<'[', '{'>,
            chars<'}', ']'>, 
            char2type<'1'>, char2type<'2'>, chars<'3','4','5'> >> f4;
    ss.clear();
    f4.write(ss,nullptr);
    std::cout<<ss.str()<<std::endl;
}

