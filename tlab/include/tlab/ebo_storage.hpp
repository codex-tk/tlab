/**
 * @file ebo_storage.hpp
 * @author ghtak (gwonhyeong.tak@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-07-29
 * 
 * @copyright Copyright (c) 2019
 * 
 */
namespace tlab{

template<typename T,typename Empty>
class ebo_storage : private Empty {
public:
    static_assert(std::is_empty_v<Empty>);

    ebo_storage(void) {}

    explicit ebo_storage(const T& v)
        : value_(v) {}

    explicit ebo_storage(T&& v)
        : value_(std::move(v)) {}

    explicit ebo_storage(const ebo_storage& rhs)
        : value_(rhs.value_) , Empty(rhs.template get<Empty>()) {}

    template<typename U>
    U& get(void) noexcept { 
        if constexpr(std::is_same_v<T,U>) return value_;
        else return *this;
    }

    template<typename U>
    const U& get(void) const noexcept { 
        if constexpr(std::is_same_v<T,U>) return value_;
        else return *this;
    }
private:
    T value_;
};

};