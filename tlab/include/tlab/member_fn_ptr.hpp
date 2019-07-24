#ifndef __tlab_member_fn_ptr_h__
#define __tlab_member_fn_ptr_h__

namespace tlab{

    template < typename C , typename S > class member_fn_ptr;
    
    template < typename C , typename R , typename ... Ts> 
    class member_fn_ptr<C , R (Ts...)>{
    public:
        using return_type = R;
        using class_type = C;
        using pointer_type = return_type (class_type::*)(Ts...);
          
        member_fn_ptr(pointer_type ptr)
            : _ptr(ptr){}
        
        return_type operator()(C* c, Ts&& ... args){
            return (c->*_ptr)(std::forward<Ts>(args)...);
        }

        return_type operator()(C& c, Ts&& ... args){
            return (c.*_ptr)(std::forward<Ts>(args)...);
        }
    private:
        pointer_type _ptr;
    };

}

#endif