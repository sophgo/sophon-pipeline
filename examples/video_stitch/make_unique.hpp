#ifndef _MAKE_UNIQUE_HPP_
#define _MAKE_UNIQUE_HPP_
#include <type_traits>
#include <memory>

// 单一元素类模板定义
template <typename T>
using Ele = typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T> >::type;

// 变长数组类模板定义
template <typename T>
using Slice = typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0, std::unique_ptr<T>>::type;

// 定长数组类模板定义
template <typename T>
using Arr = typename std::enable_if<std::extent<T>::value != 0, void>::type;

// 支持普通指针
template <typename T, typename ... Args> inline
Ele<T> make_unique(Args && ... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// 支持动态数组
template <typename T> inline
Slice<T> make_unique(size_t size) {
    using U = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>(new U[size]);
}

// 过滤定长数组
template <typename T, typename ... Args>
Arr<T> make_unique(Args &&...) = delete;

#endif