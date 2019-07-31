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
    struct alignas(sizeof(void*)) buf_hdr{
        std::size_t size;
        // todo shared?
        // std::atomic<int> refs; or shared_ptr<void>?
        static buf_hdr* from(void* ptr){
            return static_cast<buf_hdr*>(ptr) - 1;
        }

        static void* data(buf_hdr* hdr){
            return hdr + 1;
        }

        static void* create_buf(const std::size_t sz){
            buf_hdr* p = static_cast<buf_hdr*>(operator new(sizeof(buf_hdr) + sz));
            new (p) buf_hdr{sz};
            return buf_hdr::data(p);
        }

        static void release_buf(void* ptr){
            buf_hdr* p = buf_hdr::from(ptr);
            operator delete(p);
        }
    };

    template <typename C>
    struct member_func_handler{
        C* object_ptr;
        R (C::*member_func_ptr)(Ts...);
        
        R operator()(Ts&& ... args) const {
            return (object_ptr->*member_func_ptr)(std::forward<Ts>(args)...);
        }
    };

    template <typename C>
    struct const_member_func_handler{
        const C* object_ptr;
        R (C::*member_func_ptr)(Ts...) const;
        R operator()(Ts&& ... args) const{
            return (object_ptr->*member_func_ptr)(std::forward<Ts>(args)...);
        }
    };
public:
    enum class control_op { clone, destroy, release, size };
    using invoke_func_type = R (*)(void* ptr, Ts&& ...);
    using control_func_type = std::size_t (*)(control_op op,void*& p0,void* p1);

    func(void) : func(nullptr,nullptr,nullptr)
    {
    }

    func(void* p,invoke_func_type i,control_func_type c)
        : ptr_(p),invoke_(i),control_(c)
    {
    }
    
    explicit func(const func& rhs) 
        : ptr_(nullptr),invoke_(rhs.invoke_),control_(rhs.control_)
    {
        if(control_){
            control_(control_op::clone,ptr_,rhs.ptr_);
        } else {
            ptr_ = rhs.ptr_;
        }
    }

    explicit func(func&& rhs) : func()
    {
        swap(rhs);
    }

    template <typename T,
        typename = std::enable_if_t<!std::is_same_v<func,std::decay_t<T>>>>
    func(T&& t)
        : ptr_(nullptr),invoke_(nullptr),control_(nullptr)
    {
        using handler_type = std::decay_t<T>;
        if constexpr(sizeof(handler_type) <= sizeof(void*)){
            new (&ptr_) handler_type(std::forward<T>(t));
            invoke_ = invoke_handler_static<handler_type>;
            control_ = control_static<handler_type>;
        } else {
            ptr_ = buf_hdr::create_buf(sizeof(handler_type));
            new (ptr_) handler_type(std::forward<T>(t));
            invoke_ = invoke_handler_dynamic<handler_type>;
            control_ = control_dynamic<handler_type>;
        }
    }

    template <typename C>
    func(C* c,R (C::*member_func_ptr)(Ts...))
        : func(member_func_handler<C>{c,member_func_ptr})
    {
    }

    template <typename C>
    func(const C* c,R (C::*member_func_ptr)(Ts...) const)
        : func(const_member_func_handler<C>{c,member_func_ptr})
    {
    }

    ~func(void) noexcept {
        if(control_){
            control_(control_op::destroy,ptr_,nullptr);
            control_(control_op::release,ptr_,nullptr);
        }
        ptr_ = nullptr;
        invoke_ = nullptr;
        control_ = nullptr;
    }

    func& operator=(const func& rhs){
        if (*this == rhs)
            return *this;

        std::size_t bsz = control_ ? control_(control_op::size,ptr_,nullptr) : 0;
        std::size_t rbsz = rhs.control_ ? rhs.control_(control_op::size,rhs.ptr_,nullptr) : 0;
        if(bsz != 0 && rbsz != 0) {
            control_(control_op::destroy,ptr_,nullptr);
            if(bsz < rbsz){
                control_(control_op::release,ptr_,nullptr);
            }
            rhs.control_(control_op::clone,ptr_,rhs.ptr_);
            invoke_ = rhs.invoke_;
            control_ = rhs.control_;
        } else {
            func temp(rhs);
            swap(temp);
        }
        return *this;
    }

    func& operator=(func&& rhs){
        swap(rhs);
        return *this;
    }

    template <typename T,
        typename = std::enable_if_t<!std::is_same_v<func,std::decay_t<T>>>>
    func& operator=(T&& t){
        using handler_type = std::decay_t<T>;
        std::size_t bsz = control_ ? control_(control_op::size,ptr_,nullptr) : 0;
        if (bsz >= sizeof(handler_type)){
            control_(control_op::destroy,ptr_,nullptr);
            new (ptr_) handler_type(std::forward<handler_type>(t));
            invoke_ = invoke_handler_dynamic<handler_type>;
            control_ = control_dynamic<handler_type>;
        } else {
            func temp(std::forward<T>(t));
            swap(temp);
        }
        return *this;
    }

    R operator()(Ts... args) const {
        return invoke_(ptr_,std::forward<Ts>(args)...);
    }

    void swap(func& rhs){
        std::swap(ptr_,rhs.ptr_);
        std::swap(invoke_,rhs.invoke_);
        std::swap(control_,rhs.control_);
    }

    explicit operator bool() const { 
        return invoke_ != nullptr; 
    }

    bool operator == (const func& rhs){
        if( ptr_ == rhs.ptr_ && 
            invoke_ == rhs.invoke_ && 
            control_ == rhs.control_ ){
            return true;
        }
        return false;
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

    template <typename C,R (C::*member_func_ptr)(Ts...) const>
    static func make_func(const C* c){
        return { const_cast<C*>(c),invoke_const_member_func_ptr<C,member_func_ptr>,nullptr};
    }
    
    template <typename T, 
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>,func>>>
    static func make_func(T&& t){
        return func(std::forward<T>(t));
    }
private:
    template <R (*func_ptr)(Ts...)>
    static R invoke_func_ptr(void* ,Ts&&... args){
        return func_ptr(std::forward<Ts>(args)...);
    }

    template <typename C,R (C::*member_func_ptr)(Ts...)>
    static R invoke_member_func_ptr(void* ptr,Ts&&... args){
        return (static_cast<C*>(ptr)->*member_func_ptr)(std::forward<Ts>(args)...);
    }

    template <typename C,R (C::*member_func_ptr)(Ts...) const>
    static R invoke_const_member_func_ptr(void* ptr,Ts&&... args){
        return (static_cast<const C*>(ptr)->*member_func_ptr)(std::forward<Ts>(args)...);
    }

    template <typename T>
    static R invoke_handler_static(void* ptr,Ts&&... args){
        return (*static_cast<T*>(reinterpret_cast<void*>(&ptr)))(std::forward<Ts>(args)...);
    }

    template <typename T>
    static std::size_t control_static(control_op op,void*& p0,void* p1){
        switch (op) {
        case control_op::destroy:
            static_cast<T*>(reinterpret_cast<void*>(&p0))->~T();
            break;
        case control_op::clone:
            new (&p0) T(*static_cast<T*>(reinterpret_cast<void*>(&p1)));
            break;
        default:
            break;
        }
        return 0;
    }

    template <typename T>
    static R invoke_handler_dynamic(void* ptr,Ts&&... args){
        T* p = static_cast<T*>(ptr);
        return (*p)(std::forward<Ts>(args)...);
    }

    template <typename T>
    static std::size_t control_dynamic(control_op op,void*& p0,void* p1){
        switch (op) {
        case control_op::destroy:
            static_cast<T*>(p0)->~T();
            break;
        case control_op::clone:
            if(p0 == nullptr) 
                p0 = buf_hdr::create_buf(sizeof(T));
            new (p0) T(std::forward<T>(*static_cast<T*>(p1)));
            break;
        case control_op::size:
            return buf_hdr::from(p0)->size;
        case control_op::release:
            if(p0 != nullptr) 
                buf_hdr::release_buf(p0);
            p0 = nullptr;
            break;
        }
        return 0;
    }
private:
    void* ptr_;
    invoke_func_type invoke_;
    control_func_type control_;
};

} // namespace tlab


#endif