#include "catch2/catch.hh"
#include <tlab/mp.hpp>

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
    

    std::cout << typeid(tlab::pop_back<tlab::type_list<double,short,int>>::type{}).name();
}