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

    template < std::size_t ... S > struct index_sequence {};

    template < std::size_t N , std::size_t ... S> struct make_index_sequence
        : make_index_sequence< N - 1 , N - 1 , S ... >{};
    
    template < std::size_t ... S> struct make_index_sequence<0,S...>{
        using type = index_sequence<S...>;
    };  
    
}

template < typename ... Ts > struct type_list{};




}

#endif