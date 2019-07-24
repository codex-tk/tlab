/**
 * @file mp.hpp
 * @author ghtak (gwonhyeong.tak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-07-23
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef __tlab_mp_h__
#define __tlab_mp_h__

#include <iostream>

namespace tlab{
namespace internal{

    // https://en.cppreference.com/w/cpp/utility/integer_sequence
    template < std::size_t ... S > struct index_sequence {};

    template < std::size_t N , std::size_t ... S> struct make_index_sequence
        : make_index_sequence< N - 1 , N - 1 , S ... >{};
    
    template < std::size_t ... S> struct make_index_sequence<0,S...>{
        using type = index_sequence<S...>;
    };  
    
    template < typename S1 , typename S2 > struct _merge;

    template < std::size_t ... S1 , std::size_t ... S2 > 
    struct _merge< index_sequence<S1...> , index_sequence<S2...> > {
        using type = index_sequence<S1..., (sizeof...(S1) + S2)...>;
    };

    template < std::size_t N >
    struct make_index_sequence0 :
        _merge< typename make_index_sequence0<N/2>::type ,
                typename make_index_sequence0<N-N/2>::type > {};
    
    template <>  struct make_index_sequence0<0>{
        using type = index_sequence<>;
    };

    template <>  struct make_index_sequence0<1>{
        using type = index_sequence<0>;
    };
}

template < typename ... Ts > struct type_list{
    template < template <typename ...> class T > 
    struct rebind { using type = T< Ts...>;};
};


template < typename T , typename ... Us > struct push_back;

template < template < typename ... > class T , typename ... Ts , typename ... Us >
struct push_back<T<Ts...>,Us...> { using type = T<Ts...,Us...>; };


template < typename T , typename ... Us > struct push_front;

template < template < typename ... > class T , typename ... Ts , typename ... Us >
struct push_front<T<Ts...>,Us...> { using type = T<Us...,Ts...>; };


template < std::size_t I , typename T > struct at;

template < std::size_t I , template < typename ... > class T , typename Head , typename ... Ts > 
struct at<I,T<Head,Ts...>>{
    using type = typename at<I-1,T<Ts...>>::type;
};

template <template < typename ... > class T , typename Head , typename ... Ts > 
struct at<0,T<Head,Ts...>>{
    using type = Head;
};


template < std::size_t I , typename T , typename U> struct _remove_at;

template < std::size_t I , 
            template < typename ... > class T , 
            typename Type , 
            typename ... Ts ,
            typename ... Us >
struct _remove_at<I,T<Ts...>,T<Type, Us...>> {
    using type = typename _remove_at<I-1,T<Ts...,Type>,T<Us...>>::type;
};

template < template < typename ... > class T ,
            typename Type , 
            typename ... Ts , 
            typename ... Us >
struct _remove_at<0,T<Ts...>,T<Type, Us...>> {
    using type = T<Ts...,Us...>;
};


template < std::size_t I , typename T > struct remove_at;

template < std::size_t I , template < typename ... > class T , typename ... Ts > 
struct remove_at<I,T<Ts...>>{
    using type = typename _remove_at<I,T<>,T<Ts...>>::type;
};


template < typename T > struct pop_back;

template < template < typename ... > class T , typename ... Ts > 
struct pop_back<T<Ts...>>{
    using type = typename remove_at< sizeof...(Ts) - 1 ,T<Ts...>>::type;
};


template < typename T > struct pop_front;

template < template < typename ... > class T , typename ... Ts > 
struct pop_front<T<Ts...>>{
    using type = typename remove_at< 0 ,T<Ts...>>::type;
};

}

#endif