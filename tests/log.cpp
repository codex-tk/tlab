#include "catch2/catch.hh"
#include <sstream>
#include <iostream>
#include <tlab/mp.hpp>
#include <cstdarg>
#include <fstream>
#include <iomanip>

#if !defined(_WIN32) || defined(__WIN32__)
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <tlab/threading_model.hpp>

namespace tlab::log{ 
namespace expr{

template<char C> struct char2type{};

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

template <typename ... Ts> struct exprs{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){
        (dispatch<Ts>::invoke(s,c), ...);
    }
};

template <typename ... Ts >
using square_bracket_wrap = wrap_each< char2type<'['>, char2type<']'>, Ts... >;

struct timestamp{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& ){
        auto tp = std::chrono::system_clock::now();
        std::time_t now = std::chrono::system_clock::to_time_t(tp);
        struct std::tm tm;
#if defined(_WIN32) || defined(__WIN32__)
        localtime_s(&tm, &now);
#else
        localtime_r(&now, &tm);
#endif
        char time_str[32] = {0,};
        snprintf(time_str, 32, "%04d-%02d-%02d %02d:%02d:%02d.%06d", 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                static_cast<int>(
                    std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1000000));
        s << time_str;
    }
};

struct level {
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){ s << level_code(c); }
};

struct tag {
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){ s << c.tag; }
};

struct endl {
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& ){ s << "\n"; }
};

struct message{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){ s << c.msg; }
};

struct file{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){ 
        std::string path(c.file);
        s << path.substr(path.find_last_of("/\\") + 1);
        //s << c.file; 
    }
};

struct function{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){ s << c.function; }
};

struct line{
    template < typename Stream , typename Context >
    static void write(Stream&& s, Context&& c){ s << c.line; }
};

struct color{
    struct begin {
        template <typename Stream, typename Context>
        static void write(Stream &&s, Context &&c) {
            s << "\033[" << color_code(c) << "m";
        }
    };
    struct end {
        template <typename Stream, typename Context>
        static void write(Stream &&s, Context &&) {
            s << "\033[0m";
        }
    };
};

}

enum class level{ trace , debug , info ,  warn , error , fatal };

struct pp_info{
    const char *file;
    const char *function;
    int line;
};

struct basic_context{
    level level;
    const char *file;
    const char *function;
    int line;
    const char *tag;
    const char *msg;
};

int color_code(const basic_context& ctx){
    switch(ctx.level){
    case level::trace: return 32;
    case level::debug: return 33;
    case level::info: return 36;
    case level::warn: return 35;
    case level::error: return 31;
    case level::fatal: return 34;
    }
    return 0;
}

char level_code(const basic_context& ctx){
    switch(ctx.level){
    case level::trace: return 'T';
    case level::debug: return 'D';
    case level::info: return 'I';
    case level::warn: return 'W';
    case level::error: return 'E';
    case level::fatal: return 'F';
    }
    return '?';
}
template <typename U, typename T> class manager;

template <typename ThreadingModel, typename ... Ts>
class manager<ThreadingModel,tlab::type_list<Ts...>> 
    : public ThreadingModel::lock_type {
public:
    static manager& instance(void){
        static manager m;
        return m;
    }

    template <std::size_t I>
    void set_service(typename tlab::at<I,tlab::type_list<Ts...>>::type&& t){
        std::get<I>(services_) = std::move(t);
    }

    void log(level lv , pp_info&& pp_info, const char* tag, const char* msg, ...){
        char buff[4096] = {0,};
        va_list args;
        va_start(args, msg);
        vsnprintf(buff,4096,msg,args);
        va_end(args);
        send(basic_context{ lv , pp_info.file , pp_info.function , pp_info.line , tag , buff} ,
            typename tlab::internal::make_index_sequence<sizeof...(Ts)>::type{});
    }

    template <typename T, std::size_t ... S >
    void send(T&& t, tlab::internal::index_sequence<S...>&&){
        std::lock_guard<manager> guard(*this);
        ((std::get<S>(services_).log(t)),...);
    }
private:
    std::tuple<Ts...> services_;
private:
    manager(void){}
    ~manager(void){}
};

template <typename F,typename B,typename T> class service;

template <typename F,typename B,typename ... Ts> class service<F,B,tlab::type_list<Ts...>>{
public:
    using buffer_type = B;
    service(void){}

    service(Ts&&... args)
        : outputs_(std::forward<Ts>(args)...) {}

    service& operator=(service&& rhs){
        outputs_ = std::move(rhs.outputs_);
        return *this;
    }

    template <typename T>
    void log(const T& t){
        buffer_type buff;
        F::format(buff,t);
        send(buff,typename tlab::internal::make_index_sequence<sizeof...(Ts)>::type{});
    }

    template <typename T, std::size_t ... S >
    void send(T&& t, tlab::internal::index_sequence<S...>&&){
        ((std::get<S>(outputs_).write(t)),...);
    }
private:
    std::tuple<Ts...> outputs_;
};

struct ostringstream_buffer{
    std::ostringstream oss;
    template <typename T> ostringstream_buffer &operator<<(T &&t) {
        oss << t;
        return *this;
    }
};

template <typename T>
T& operator<<(T& t,const ostringstream_buffer& buf){
    t << buf.oss.str();
    return t;
};

struct sprintf_buffer{
    char data[1024] = {0,};
    int len = 0;

    sprintf_buffer &operator<<(const char c) {
        len += std::snprintf(data+len,1024-len,"%c",c);
        return *this;
    }

    sprintf_buffer &operator<<(const char* p) {
        len += std::snprintf(data+len,1024-len,"%s",p);
        return *this;
    }

    sprintf_buffer &operator<<(const std::string& str) {
        len += std::snprintf(data+len,1024-len,"%s",str.c_str());
        return *this;
    }

    sprintf_buffer &operator<<(int v) {
        len += std::snprintf(data+len,1024-len,"%d",v);
        return *this;
    }
};

template <typename T>
T& operator<<(T& t,const sprintf_buffer& buf){
    t << buf.data;
    return t;
};

struct cout_output {
    template < typename Buf>
    void write(const Buf& b){
        std::cout << b;
    }
};

class file_output{
public:
    file_output(void)
        : path_("./"), max_lines_(0xffff), remain_days_(30),
        lines_(0), file_seq_(0)
    {}

    file_output(const std::string& path , 
                const int remain_days = 30,
                const int max_lines = 0xffff)
        : path_(path), max_lines_(max_lines), remain_days_(remain_days),
        lines_(0), file_seq_(0){
         if (dir_exist(path_) == false) {
            mkdir(path_);
        };
    }

    template < typename Buf >
    void write(const Buf& b ){
        if (ofstream_.is_open() == false) {
            std::ostringstream oss;
            int64_t now_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
            std::time_t now = now_ms / 1000;
            struct std::tm tm;
    #if defined(_WIN32) || defined(__WIN32__)
            localtime_s(&tm, &now);
    #else
            localtime_r(&now, &tm);
    #endif
            oss << path_ << '/' << std::setfill('0') << std::setw(4)
                << tm.tm_year + 1900 << std::setw(2) << tm.tm_mon + 1
                << std::setw(2) << tm.tm_mday << '_' << std::setw(2) << tm.tm_hour
                << std::setw(2) << tm.tm_min << std::setw(2) << tm.tm_sec << '_'
                << std::setw(4) << ++file_seq_;
            ofstream_.open(oss.str());
        }

        if (ofstream_.good()) {
            ofstream_ << b;
            ++lines_;
        }
        if (lines_ >= max_lines_) {
            ofstream_.close();
            lines_ = 0;
            delete_old_logs();
        }
    }

    void delete_old_logs(void) {
        std::time_t now;
        struct std::tm tm;

        std::time(&now);
        now -= (60 * 60 * 24 * remain_days_);
    #if defined(_WIN32) || defined(__WIN32__)
        localtime_s(&tm, &now);
    #else
        localtime_r(&now, &tm);
    #endif
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << tm.tm_year + 1900
            << std::setw(2) << tm.tm_mon + 1 << std::setw(2) << tm.tm_mday << '_'
            << std::setw(2) << tm.tm_hour << std::setw(2) << tm.tm_min
            << std::setw(2) << tm.tm_sec;
        std::string base = oss.str();
        const std::size_t base_len = base.length();
        std::vector<std::string> files;
        list_files(path_, files);
        for (std::string &file : files) {
            if (file.length() > base_len) {
                if (file.substr(0, base_len) < base) {
    #if defined(_WIN32) || defined(__WIN32__)
                    std::string full_path = _path + '\\' + file;
                    DeleteFileA(full_path.c_str());
    #else
                    std::string full_path = path_ + '/' + file;
                    remove(full_path.c_str());
    #endif
                }
            }
        }
    }
private:
    bool dir_exist(const std::string &path) {
    #if defined(_WIN32) || defined(__WIN32__)
        DWORD dwAttrib = GetFileAttributesA(path.c_str());

        return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    #else
        struct stat info;
        if (stat(path.c_str(), &info) != 0)
            return false;
        else if (info.st_mode & S_IFDIR)
            return true;
        else
            return false;
    #endif
    }

    bool mkdir(const std::string &path) {
    #if defined(_WIN32) || defined(__WIN32__)
        LPSECURITY_ATTRIBUTES attr = nullptr;
        return CreateDirectoryA(path.c_str(), attr) == TRUE;
    #else
        return ::mkdir(path.c_str(), 0755) == 0;
    #endif
    }

    void list_files(const std::string &path, std::vector<std::string> &files) {
    #if defined(_WIN32) || defined(__WIN32__)
        WIN32_FIND_DATAA find_data;
        HANDLE h = FindFirstFileA((path + "\\*.*").c_str(), &find_data);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    files.emplace_back(find_data.cFileName);
                }
            } while (FindNextFileA(h, &find_data) == TRUE);
            FindClose(h);
        }
    #else
        struct stat statinfo;
        DIR *dir = opendir(path.c_str());
        if (dir == nullptr)
            return;

        char full_path[4096];
        struct dirent *ent = nullptr;
        while ((ent = readdir(dir)) != nullptr) {
            sprintf(full_path, "%s/%s", path.c_str(), ent->d_name);
            if (stat(full_path, &statinfo) == 0) {
                if (!S_ISDIR(statinfo.st_mode)) {
                    files.emplace_back(ent->d_name);
                }
            }
        }
        closedir(dir);
    #endif
    }
private:
    std::string path_;
    int max_lines_;
    int remain_days_;
    std::ofstream ofstream_;
    int lines_;
    int file_seq_;
};

}

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

using _loglvl = tlab::log::level;

#ifndef DEFINE_TLOGGER
#define DEFINE_TLOGGER(_class) _class& logger_instance(void){ return _class::instance(); }
#endif

#ifndef TLAB_LOG
#if defined(_WIN32) || defined(__WIN32__)
#define TLOG_T(_tag,_message, ...) TLOG(logger_instance(), _loglvl::trace, _tag,_message, __VA_ARGS__)
#define TLOG_D(_tag,_message, ...) TLOG(logger_instance(), _loglvl::debug, _tag,_message, __VA_ARGS__)
#define TLOG_I(_tag,_message, ...) TLOG(logger_instance(), _loglvl::info, _tag,_message, __VA_ARGS__)
#define TLOG_W(_tag,_message, ...) TLOG(logger_instance(), _loglvl::warn, _tag,_message, __VA_ARGS__)
#define TLOG_E(_tag,_message, ...) TLOG(logger_instance(), _loglvl::error, _tag,_message, __VA_ARGS__)
#define TLOG_F(_tag,_message, ...) TLOG(logger_instance(), _loglvl::fatal, _tag,_message, __VA_ARGS__)
#else
#define TLOG_T(_tag,_message, ...) TLOG(logger_instance(), _loglvl::trace, _tag,_message, ##__VA_ARGS__)
#define TLOG_D(_tag,_message, ...) TLOG(logger_instance(), _loglvl::debug, _tag,_message, ##__VA_ARGS__)
#define TLOG_I(_tag,_message, ...) TLOG(logger_instance(), _loglvl::info, _tag,_message, ##__VA_ARGS__)
#define TLOG_W(_tag,_message, ...) TLOG(logger_instance(), _loglvl::warn, _tag,_message, ##__VA_ARGS__)
#define TLOG_E(_tag,_message, ...) TLOG(logger_instance(), _loglvl::error, _tag,_message, ##__VA_ARGS__)
#define TLOG_F(_tag,_message, ...) TLOG(logger_instance(), _loglvl::fatal, _tag,_message, ##__VA_ARGS__)
#endif
#endif

using namespace tlab::log;
using namespace tlab::log::expr;

TEST_CASE("Log" , "Format"){
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
    struct context { const char* tag; };
    static_formatter< square_bracket_wrap<
            char2type<'1'>, tag , chars<'3','4','5'> >>::format(ss,context{"tag"});
    REQUIRE(ss.str() == "[1][tag][345]");
    ss.str("");
}

using console_service_type = tlab::log::service<
        static_formatter<
            color::begin ,
            square_bracket_wrap<
                timestamp, tag, expr::level, message,
                exprs<file, chars<':'>, line>
            > , 
            color::end, 
            endl
        >
        ,sprintf_buffer
        ,tlab::type_list<cout_output>>;

using file_service_type = tlab::log::service<
        static_formatter<
            square_bracket_wrap<
                timestamp, tag, expr::level, message,
                exprs<file, chars<':'>, line>
            > , 
            endl
        >
        ,sprintf_buffer
        ,tlab::type_list<file_output>>;

using logger = tlab::log::manager< 
    tlab::multi_threading_model,
    tlab::type_list<console_service_type/*,file_service_type */>>;

DEFINE_TLOGGER(logger)

TEST_CASE("log" , "simple"){
    //logger::instance().set_service<1>(file_service_type{file_output{ "./logs" }});
    TLOG_T("tk","trace");
    TLOG_D("tk","debug");
    TLOG_I("tk","info");
    TLOG_W("tk","warn");
    TLOG_E("tk","error");
    TLOG_F("tk","fatal");
}
