#ifndef __tlab_member_func_ptr_h__
#define __tlab_member_func_ptr_h__

namespace tlab{

    template < typename C , typename S > class member_func_ptr;
    
    template < typename C , typename R , typename ... Ts> 
    class member_func_ptr<C , R (Ts...)>{
    public:
        using return_type = R;
        using class_type = C;
        using pointer_type = return_type (class_type::*)(Ts...);
          
        member_func_ptr(pointer_type ptr)
            : ptr_(ptr){}
        
        return_type operator()(C* c, Ts&& ... args){
            return (c->*ptr_)(std::forward<Ts>(args)...);
        }

        return_type operator()(C& c, Ts&& ... args){
            return (c.*ptr_)(std::forward<Ts>(args)...);
        }
    private:
        pointer_type ptr_;
    };

}

#endif