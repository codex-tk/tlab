#ifndef __tlab_member_ptr_h__
#define __tlab_member_ptr_h__

namespace tlab{

    template < typename C , typename E >
    class member_ptr{ 
    public:
        using element_type = E;
        using class_type = C;
        using pointer_type = element_type class_type::*;
          
        member_ptr(pointer_type ptr)
            : _ptr(ptr){}
        
        element_type get(class_type& c){
            return (c.*_ptr);
        }
        
        void set(class_type& c, element_type&& v){
            (c.*_ptr) = std::forward<element_type>(v);
        }

        element_type get(class_type* c){
            return (c->*_ptr);
        }

        void set(class_type* c, element_type&& v){
            (c->*_ptr) = std::forward<element_type>(v);
        }
    private:
        pointer_type _ptr;
    };

}

#endif