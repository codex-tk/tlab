/**
 * @file error.hpp
 * @author ghtak (gwonhyeong.tak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-07-24
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef __tlab_error_h__
#define __tlab_error_h__

#include <system_error>

namespace tlab {
/**
 * @brief
 *
 */
enum class errc : int {
    success = 0,
    fail = 1,
};

/**
 * @brief DL Lookup 으로 std 에서 해당 함수를 찾아서 사용
 *  tlab::errc 와 동일한 네임스페이스에 존재해야한다.
 *  template <> struct is_error_condition_enum<tlab::errc> : true_type {};
 *
 * @param ec
 * @return std::error_code
 */
std::error_condition make_error_condition(tlab::errc ec);

} // namespace tlab

namespace std {
    template <> struct is_error_condition_enum<tlab::errc> : true_type {};
    //template <> struct is_error_code_enum<tlab::errc> : true_type {};
} // namespace std

#endif