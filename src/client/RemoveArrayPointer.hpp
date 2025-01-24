#pragma once

namespace std {
    template<typename T> struct remove_array_pointer { using type = T; };
    template<typename T> using remove_array_pointer_t = typename remove_array_pointer<T>::type;
    template<typename T, size_t S> struct remove_array_pointer<T[S]> { using type = T; };
}