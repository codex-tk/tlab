/**
 * @file log.hpp
 * @author ghtak (gwonhyeong.tak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-07-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef __tlab_log_h__
#define __tlab_log_h__

#include <tlab/mp.hpp>

#include <mutex>

namespace tlab{
namespace log{

enum class level{ trace , debug , info ,  warn , error , fatal };

struct pp_info{
    const char *file;
    const char *function;
    int line;
    pp_info(const char *file, const char *func, int line)
        : file(file), function(func), line(line) {}
    pp_info(pp_info&& rhs)
        :file(rhs.file), function(rhs.function), line(rhs.line) {}
};

struct basic_context{
    level level;
    pp_info pp_info;
    const char *tag;
    const char *msg;
};

struct single_thread_model{
    struct null_lock{};
    template < typename T > struct null_lock_guard{
        null_lock_guard(null_lock&){}
    };

    using lock_type = null_lock;
    template <typename T> 
    using lock_guard = null_lock_guard<T>;
};

struct multi_thread_model{
    using lock_type = std::mutex;
    template <typename T>
    using lock_guard = std::lock_guard<T>;
};
template <typename S , typename T = multi_thread_model > class basic_logger;

template <typename ThreadingModel , typename ... Services>
class basic_logger<tlab::type_list<Services...>,ThreadingModel> {
public:
    using sequence_type = typename tlab::internal::make_index_sequence<sizeof...(Services)>::type;
    using lock_type = typename ThreadingModel::lock_type;
    using lock_guard = typename ThreadingModel::template lock_guard<lock_type>;

    void log(level lv , pp_info&& pp_info, const char* tag, const char* msg, ...);

    template < typename T >
    struct dispatch;
    
    template < std::size_t ... S >
    struct dispatch<tlab::internal::index_sequence<S...>> {
        template < typename C >
        static void invoke(std::tuple<Services...>& tuple , C&& c){
            std::cout << c.pp_info.file << std::endl;
            ((std::cout<<std::get<S>(tuple)),...);
        }
    };

    template < std::size_t I> 
    typename tlab::at<I,tlab::type_list<Services...>>::type& service(void) {
        return std::get<I>(services_);
    } 
private:
    lock_type lock_;
    std::tuple<Services...> services_;
};

template <typename ThreadingModel , typename ... Services>
void basic_logger<tlab::type_list<Services...>,ThreadingModel>::log(
    level lv, 
    log::pp_info&& pp_info, 
    const char* tag, 
    const char* msg, ...)
{
    char buff[4096] = {0,};
    va_list args;
    va_start(args, msg);
    vsnprintf(buff,4096,msg,args);
    va_end(args);
    basic_logger::lock_guard guard(lock_);
    dispatch<sequence_type>::invoke(services_, 
        basic_context{ lv , std::forward<log::pp_info>(pp_info), tag , buff});
}

template < typename ... Services >
using logger = basic_logger< tlab::type_list< Services...>>;

template < typename Formatter , typename Ostream >
class basic_service;

template < typename Formatter , typename ... Ostreams >
class basic_service<Formatter, tlab::type_list<Ostreams...>> {
public:
    basic_service(void){}
    ~basic_service(void){}
private:
    std::tuple<Ostreams...> ostreams_;
};

} // namespace log
} // namespace tlab

#ifndef TLOG_PP_INFO
#define TLOG_PP_INFO tlab::log::pp_info(__FILE__, __FUNCTION__, __LINE__)
#endif

#ifndef TLOG
#if defined(_WIN32) || defined(__WIN32__)
#define TLOG(_logger,_lv,_tag,_msg,...)\
do { _logger.log(_lv, TLOG_PP_INFO, _tag, _msg, __VA_ARGS__ );} while(0)
#else
#define TLOG(_logger,_lv,_tag,_msg,...)\
do { _logger.log(_lv, TLOG_PP_INFO, _tag, _msg, ##__VA_ARGS__ );} while(0)
#endif
#endif
#endif