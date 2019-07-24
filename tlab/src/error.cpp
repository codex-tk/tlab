#include <iostream>
#include <tlab/error.hpp>

namespace tlab {

//http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-1.html

class error_category_impl : public ::std::error_category {
public:
    error_category_impl(void) noexcept {}

    virtual const char *name() const noexcept override {
        return "tlab_error_category";
    }

    virtual ::std::string message(int code) const override {
        switch(static_cast<tlab::errc>(code)){
        case tlab::errc::success: 
            return {"success"};
        case tlab::errc::fail:
            return {"fail"};
        }
        return {"unknown"};
    }

    virtual bool equivalent(const std::error_code& code,  int condition) 
        const noexcept override {
        switch(static_cast<tlab::errc>(condition)){
        case tlab::errc::success: 
            return code.value() == 0;
        case tlab::errc::fail:
            return code.value() != 0;
        }
        return false;
    }

    static error_category_impl &instance(void) {
        static error_category_impl category;
        return category;
    }
};

std::error_condition make_error_condition(tlab::errc ec) {
    return std::error_condition(
        static_cast<int>(ec), tlab::error_category_impl::instance());
}

} // namespace tlab
