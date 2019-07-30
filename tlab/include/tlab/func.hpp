/**
 * @file func.hpp
 * @author ghtak (gwonhyeong.tak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-07-29
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef __tlab_func_h__
#define __tlab_func_h__

#include <iostream>
#include <tlab/ebo_storage.hpp>

namespace tlab{

template <typename Signature> class func;

template <typename R, typename ... Ts > 
class func<R (Ts...)>{
public:
    enum class control_op { clone , destroy };
    using invoke_func_type = R (*)(void* ptr, Ts&& ...);
    using control_func_type = void (*)(control_op op,void*& p0,void* p1);

    func(void* ptr,invoke_func_type i,control_func_type c)
        : ptr_(ptr),invoke_(i),control_(c)
    {
    }

    ~func(void) noexcept {
        if(control_){
            control_(control_op::destroy,ptr_,nullptr);
        }
        ptr_ = nullptr;
        invoke_ = nullptr;
        control_ = nullptr;
    }

    explicit func(const func& rhs) 
        : ptr_(nullptr),invoke_(nullptr),control_(nullptr)
    {
        if(rhs.control_){
            rhs.control_(control_op::clone,ptr_,rhs.ptr_);
        } else {
            ptr_ = rhs.ptr_;
        }
        invoke_ = rhs.invoke_;
        control_ = rhs.control_;
    }

    explicit func(func&& rhs) = default;

    template <typename T,
        typename = std::enable_if_t<!std::is_same_v<func,std::decay_t<T>>>>
    explicit func(T&& t)
        : ptr_(nullptr),invoke_(nullptr),control_(nullptr)
    {
        using handler_type = std::decay_t<T>;
        if constexpr(sizeof(handler_type) <= sizeof(ptr_)){
            new (&ptr_) handler_type(std::forward<T>(t));
            invoke_ = invoke_handler_local<handler_type>;
            control_ = control_local<handler_type>;
        } else {
            //todo
        }
    }

    func& operator=(const func& rhs){
        func temp(rhs);
        swap(temp);
        return *this;
    }

    func& operator=(func&& rhs){
        swap(rhs);
        return *this;
    }

    R operator()(Ts&&... args){
        return invoke_(ptr_,std::forward<Ts>(args)...);
    }

    void swap(func& rhs){
        std::swap(ptr_,rhs.ptr_);
        std::swap(invoke_,rhs.invoke_);
        std::swap(control_,rhs.control_);
    }
public:
    template <R (*func_ptr)(Ts...)>
    static func make_func(void){
        return {nullptr,invoke_func_ptr<func_ptr>,nullptr};
    }

    template <typename C,R (C::*member_func_ptr)(Ts...)>
    static func make_func(C* c){
        return {c,invoke_member_func_ptr<C,member_func_ptr>,nullptr};
    }
    
    template <typename T, 
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>,func>>>
    static func make_func(T&& t){
        return func(std::forward<T>(t));
    }
private:
    template<R (*func_ptr)(Ts...)>
    static R invoke_func_ptr(void* ,Ts&&... args){
        return func_ptr(std::forward<Ts>(args)...);
    }

    template<typename C,R (C::*member_func_ptr)(Ts...)>
    static R invoke_member_func_ptr(void* ptr,Ts&&... args){
        return (static_cast<C*>(ptr)->*member_func_ptr)(std::forward<Ts>(args)...);
    }

    template<typename T>
    static R invoke_handler_local(void* ptr,Ts&&... args){
        return (*static_cast<T*>(reinterpret_cast<void*>(&ptr)))(std::forward<Ts>(args)...);
    }

    template<typename T>
    static void control_local(control_op op,void*& p0,void* p1){
        if(op == control_op::destroy){
            static_cast<T*>(reinterpret_cast<void*>(&p0))->~T();
        } else {
            new (&p0) T(*static_cast<T*>(reinterpret_cast<void*>(&p1)));
        }
    }

private:
    void* ptr_;
    invoke_func_type invoke_;
    control_func_type control_;
};

/* 
template <typename R, typename ... Ts > 
class func<R (Ts...)>{
private:
    struct callable_base{
        using invoke_type =  R (*)(callable_base* callable, Ts&& ...);
        using destroy_type = void (*)(callable_base* callable);
        using clone_type = callable_base* (*)(const callable_base* callable);

        invoke_type invoke;
        destroy_type destroy;
        clone_type clone;

        callable_base(invoke_type i,destroy_type d,clone_type c)
            : invoke(i), destroy(d), clone(c) {}
    };

    template <typename T>
    class callable : public callable_base{
    public:
        explicit callable(const T& h) : 
            callable_base(&callable::invoke_impl,
                        &callable::destroy_impl,
                        &callable::clone_impl), 
            handler(h) {}

        explicit callable(T&& h) :
            callable_base(&callable::invoke_impl,
                        &callable::destroy_impl,
                        &callable::clone_impl), 
            handler(std::move(h)) {}
    private:
        T handler;
    private:
        static R invoke_impl(callable_base* p, Ts&& ... args){
            callable* pthis = static_cast<callable*>(p);
            return pthis->handler( std::forward<Ts>(args)...);
        }

        static void destroy_impl(callable_base* p){
            callable* pthis = static_cast<callable*>(p);
            delete pthis;
        }

        static callable_base* clone_impl(const callable_base* p){
            const callable* pthis = static_cast<const callable*>(p);
            return new callable(pthis->handler);
        }
    };
    template<typename T>
    using remove_cv_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;
public:
    func(void) : callable_(nullptr){}

    explicit func(const func& rhs) 
        : callable_(rhs.callable_ ?
            rhs.callable_->clone(rhs.callable_) : nullptr)
    {
        std::cout << "explicit func(const func& rhs)" << std::endl;
    }

    explicit func(func&& rhs) 
        : callable_(nullptr)
    {
        std::swap(callable_,rhs.callable_);
        std::cout << "explicit func(func&& rhs) " << std::endl;
    }

    func& operator=(const func& rhs){
        std::cout << "func& operator=(const func& rhs" << std::endl;
        func swp(rhs);
        std::swap(callable_,swp.callable_);
        return *this;
    }

    func& operator=(func&& rhs) noexcept {
        std::cout << "func& operator=(func&& rhs)" << std::endl;
        std::swap(callable_,rhs.callable_);
        return *this;
    }

    template <typename T, typename = std::enable_if_t<
            !std::is_same_v<func,remove_cv_ref_t<T>>>>
    explicit func(T&& t)
        : callable_(new callable<remove_cv_ref_t<T>>(std::forward<T>(t))) 
    {
        std::cout << "explicit func(T&& t)" << std::endl;
    }

    template <typename T, typename = std::enable_if_t<
            !std::is_same_v<func,remove_cv_ref_t<T>>>>
    func& operator=(T&& t){
        std::cout << "func& operator=(T&& t)" << std::endl;
        func swp(std::forward<T>(t));
        std::swap(callable_,swp.callable_);
        return *this;
    }

    ~func(void){
        destroy();
    }

    R operator()(Ts&&... args){
        if(callable_)
            return callable_->invoke(callable_,std::forward<Ts>(args)...);
        return R();
    }

    void destroy(void) noexcept{
        if(callable_)
            callable_->destroy(callable_);
        callable_ = nullptr;
    }

    explicit operator bool( void ) const {
        return callable_ != nullptr;
    }
private:
    callable_base* callable_;
};
*/
} // namespace tlab


#endif