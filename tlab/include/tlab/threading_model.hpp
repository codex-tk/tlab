/**
 * @file threading_mode.hpp
 * @author ghtak (gwonhyeong.tak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-07-29
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef __tlab_threading_model_h__
#define __tlab_threading_model_h__

#include <mutex>

namespace tlab{
namespace internal {
    class no_lock{
    public:
        void lock(void){}
        void unlock(void){}
        bool try_lock(void){return true;}
    };
} // namespace internal
    

    struct single_threading_model{
        using lock_type = internal::no_lock;
    };

    struct multi_threading_model{
        using lock_type = std::mutex;
    };
    
} // namespace tlab



#endif