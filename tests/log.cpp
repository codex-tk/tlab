#include "catch2/catch.hh"
#include <sstream>
#include <iostream>
#include <tlab/mp.hpp>

namespace tlab::log{ 
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

}

using namespace tlab::log;

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


#include <tlab/mp.hpp>
#include <cstdarg>
#include <mutex>


namespace tlab{
namespace log{

enum class level{ trace , debug , info ,  warn , error , fatal };

struct pp_info{
    const char *file;
    const char *function;
    int line;
};

struct basic_context{
    level level;
    pp_info pp_info;
    const char *tag;
    const char *msg;
};

template < typename T > class manager;

template <typename ... Ts>
class manager<tlab::type_list<Ts...>>{
public:
    static manager& instance(void){
        static manager m;
        return m;
    }

    template <std::size_t I , typename T = typename tlab::at<I,tlab::type_list<Ts...>>::type >
    void set_service(T&& t){
        std::get<I>(services_) = std::move(t);
    }

    void log(){
        send(
            context{"tag"}
            ,tlab::internal::make_index_sequence<sizeof...(Ts)>::type{});
    }

    template <typename T, std::size_t ... S >
    void send(T&& t, tlab::internal::index_sequence<S...>&&){
        ((std::get<S>(services_).log(t)),...);
    }

private:
    std::tuple<Ts...> services_;
private:
    manager(void){}
    ~manager(void){}
};

struct cout_output {
    template < typename Buf , typename C >
    void write(const Buf& b , const C& ){
        std::cout << b;
    }
};

template <typename F,typename T> class service;

template <typename F,typename ... Ts> class service<F,tlab::type_list<Ts...>>{
public:
    service(void){}

    service(Ts&&... args)
        : outputs_(std::forward<Ts>(args)...) {}

    service& operator=(service&& rhs){
        outputs_ = std::move(rhs.outputs_);
        return *this;
    }

    template <typename T>
    void log(const T& t){
        std::ostringstream oss;
        F::format(oss,t);
        send(oss.str(),tlab::internal::make_index_sequence<sizeof...(Ts)>::type{});
    }

    template <typename T, std::size_t ... S >
    void send(T&& t, tlab::internal::index_sequence<S...>&&){
        ((std::get<S>(outputs_).write(t,t)),...);
    }
private:
    std::tuple<Ts...> outputs_;
};

}}

TEST_CASE("log" , "simple"){
    using service_type = tlab::log::service<
        tlab::log:: static_formatter<tlab::log::square_bracket_wrap<tag>>
        ,tlab::type_list<tlab::log::cout_output>>;
    service_type svc{};
    tlab::log::manager<tlab::type_list<service_type>>::instance().set_service<0>(svc);
    tlab::log::manager<tlab::type_list<service_type>>::instance().log();
}
/*
namespace tlab{
namespace log{

enum class level{ trace , debug , info ,  warn , error , fatal };

struct pp_info{
    const char *file;
    const char *function;
    int line;
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
        null_lock_guard(T&){}
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

template <typename S , typename T = single_thread_model > class basic_logger;

template <typename ThreadingModel , typename ... Services>
class basic_logger<tlab::type_list<Services...>,ThreadingModel> {
public:
    using sequence_type = typename tlab::internal::make_index_sequence<sizeof...(Services)>::type;
    using lock_type = typename ThreadingModel::lock_type;
    using lock_guard = typename ThreadingModel::template lock_guard<lock_type>;

    void log(level lv , pp_info&& pp_info, const char* tag, const char* msg, ...){
        char buff[4096] = {0,};
        va_list args;
        va_start(args, msg);
        vsnprintf(buff,4096,msg,args);
        va_end(args);
        basic_logger::lock_guard guard(lock_);
        dispatch<sequence_type>::invoke(services_, 
            basic_context{ lv , std::forward<log::pp_info>(pp_info), tag , buff});
    }

    template < typename T > struct dispatch;
    
    template < std::size_t ... S >
    struct dispatch<tlab::internal::index_sequence<S...>> {
        template < typename C >
        static void invoke(std::tuple<Services...>& tuple , C&& c){
            std::cout << c.pp_info.file << std::endl;
            ((std::get<S>(tuple).log(c)),...);
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

template < typename ... Services >
using logger = basic_logger< tlab::type_list< Services...>>;

template < typename StaticFormatter , typename Output >
class basic_service;

template < typename StaticFormatter , typename ... Outputs >
class basic_service<StaticFormatter, tlab::type_list<Outputs...>> {
public:
    using sequence_type = typename tlab::internal::make_index_sequence<sizeof...(Outputs)>::type;
    basic_service(void){}
    ~basic_service(void){}

    template < typename T > struct dispatch;
    
    template < std::size_t ... S >
    struct dispatch<tlab::internal::index_sequence<S...>> {
        template < typename T >
        static void invoke(std::tuple<Outputs...>& tuple , T&& t){
            ((std::get<S>(tuple).log(t)),...);
        }
    };

    template < std::size_t I> 
    typename tlab::at<I,tlab::type_list<Outputs...>>::type& output(void) {
        return std::get<I>(outputs_);
    }

    template < typename C >
    void log(const C& ){
        std::stringstream ss;
        //formatter_.format(ss,c);
        dispatch<sequence_type>::invoke(outputs_,ss.str());
    }
private:
    StaticFormatter formatter_;
    std::tuple<Outputs...> outputs_;
};

class cout_output{
public:
    template < typename S > void log(S&& s){ std::cout << s; }
};

} // namespace log
} // namespace tlab

#ifndef TLOG_PP_INFO
#define TLOG_PP_INFO tlab::log::pp_info{ __FILE__, __FUNCTION__, __LINE__ }
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


TEST_CASE("log" , "simple"){

    using test_service = tlab::log::basic_service< int , tlab::type_list< tlab::log::cout_output > >;

    tlab::log::logger< test_service > logger;

    //logger.service<0>() = test_service{};

    TLOG(logger,tlab::log::level::debug,"tag","%s msg" , "test");
     
}*/
