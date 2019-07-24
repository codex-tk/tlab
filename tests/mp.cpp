#include "catch2/catch.hh"
#include <tlab/mp.hpp>
#include <sstream>

using namespace tlab::internal;

TEST_CASE("MetaProgramming_Seq" , "[Seq]") {
    static_assert(
        std::is_same<
            index_sequence<0>,
            make_index_sequence<1>::type>::value);

    static_assert(
        std::is_same<
            index_sequence<0,1,2,3,4,5,6,7,8,9,10,11>,
            make_index_sequence<12>::type>::value);

    static_assert(
        std::is_same<
            make_index_sequence0<12>::type,
            make_index_sequence<12>::type>::value);
}


TEST_CASE("MetaProgramming_TL" , "[TL]") {
    static_assert(
        std::is_same<
            tlab::push_back<tlab::type_list<int>,double>::type ,
            tlab::type_list<int,double>>::value
    );

    static_assert(
        std::is_same<
            tlab::push_back<tlab::type_list<int>,double,short>::type ,
            tlab::type_list<int,double,short>>::value
    );

    static_assert(
        std::is_same<
            tlab::push_front<tlab::type_list<int>,double,short>::type ,
            tlab::type_list<double,short,int>>::value
    );

    static_assert(
        std::is_same<
            tlab::at<0,tlab::type_list<double,short,int>>::type ,
            double>::value
    );

    static_assert(
        std::is_same<
            tlab::at<1,tlab::type_list<double,short,int>>::type ,
            short>::value
    );
    
    static_assert(
        std::is_same<
            tlab::remove_at<0,tlab::type_list<double,short,int>>::type ,
            tlab::type_list<short,int>>::value
    );

    static_assert(
        std::is_same<
            tlab::remove_at<1,tlab::type_list<double,short,int>>::type ,
            tlab::type_list<double,int>>::value
    );

    static_assert(
        std::is_same<
            tlab::remove_at<2,tlab::type_list<double,short,int>>::type ,
            tlab::type_list<double,short>>::value
    );

    static_assert(
        std::is_same<
            tlab::pop_back<tlab::type_list<double,short,int>>::type ,
            tlab::type_list<double,short>>::value
    );

    static_assert(
        std::is_same<
            tlab::pop_front<tlab::type_list<double,short,int>>::type ,
            tlab::type_list<short,int>>::value
    );
    
    static_assert(
        std::is_same<
            tlab::pop_front<tlab::type_list<double,short,int>>::type::rebind<std::tuple>::type ,
            std::tuple<short,int>>::value
    );
}

// https://en.cppreference.com/w/cpp/language/fold
/*
Syntax
( pack op ... )	1) unary right fold
( ... op pack )	2) unary left fold
( pack op ... op init )	3) binary right fold
( init op ... op pack )	4) binary left fold

op 
 any of the following 32 binary operators: 
 + - * / % ^ & | = < > << >> += -= *= /= %= ^= &= |= <<= >>= == != <= >= && || , .* ->*
 In a binary fold, both ops must be the same.
pack
 an expression that contains an unexpanded parameter pack and 
 does not contain an operator with precedence lower than cast at the top level 
 (formally, a cast-expression)
init
 an expression that does not contain an unexpanded parameter pack and does not 
 */
namespace internal{
    template < typename ... Ts >
    auto unary_right(Ts&& ... args){
        return (args + ...);
    }

    template < typename ... Ts >
    auto unary_left(Ts&& ... args){
        return (... + args);
    }

    template < typename ... Ts >
    auto binary_right(Ts&& ... args ){
        return (args + ... + (1+2));
    }

    template < typename ... Ts >
    auto binary_left(Ts&& ... args ){
        return ((1+2) + ... + args);
    }

    template < typename F , typename ... Ts >
    void comma_op(F&& f, Ts ... args ) {
        (f(std::forward<Ts>(args)),...);
    }
}

TEST_CASE("FoldExpression" , "[]") {
    REQUIRE(internal::unary_right(1,2,3,4) == (1+(2+(3+4))));
    REQUIRE(internal::unary_left(1,2,3,4) == (((1+2)+3)+4));
    REQUIRE(internal::binary_right(1,2,3,4) == (1+(2+(3+(4+(1+2))))));
    REQUIRE(internal::binary_left(1,2,3,4) == (((((1+2)+1)+2)+3)+4));
}

// https://en.cppreference.com/w/c/language/operator_other#Comma_operator
// comma op is seq point
TEST_CASE("Comma_FoldTest" , "[]") {
    std::stringstream ss;
    internal::comma_op([&](auto s){
        ss << s;
    } , 36 , " is greater then " , "24");
    REQUIRE(ss.str() == "36 is greater then 24");

    std::string comma;
    (comma.push_back('c'),
        (comma.push_back('o'),
            (comma.push_back('m'),
                (comma.push_back('m'), 
                (comma.push_back('a'))))));

    REQUIRE(comma == "comma");
}