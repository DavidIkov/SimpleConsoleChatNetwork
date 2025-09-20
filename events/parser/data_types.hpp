#pragma once
#include <cstdint>
#include <string>

namespace events::data_types {

struct INT32 {};
struct INT64 {};
struct UINT32 {};
struct UINT64 {};
struct STRING {};
struct FLOAT {};
struct BOOL {};

template <typename T>
struct StoringType;

template <>
struct StoringType<INT32> {
    using type = int32_t;
};

template <>
struct StoringType<INT64> {
    using type = int64_t;
};

template <>
struct StoringType<UINT32> {
    using type = uint32_t;
};

template <>
struct StoringType<UINT64> {
    using type = uint64_t;
};

template <>
struct StoringType<STRING> {
    using type = std::string;
};

template <>
struct StoringType<FLOAT> {
    using type = float;
};

template <>
struct StoringType<BOOL> {
    using type = bool;
};

template <typename T>
struct TypeDescription {
    using type = T;
    std::size_t GetMaxSize() const {
        return sizeof(typename StoringType<T>::type);
    }
};

template <>
struct TypeDescription<STRING> {
    using type = STRING;
    std::size_t max_len;
    std::size_t GetMaxSize() const { return max_len; }
};

template <typename T>
struct PacketData {
    explicit inline PacketData(typename StoringType<T>::type v)
        : v_(std::move(v)) {}
    typename StoringType<T>::type v_;
    std::size_t GetSize() const { return sizeof(v_); }
};

template <>
inline std::size_t PacketData<STRING>::GetSize() const {
    return v_.size() + 1;
}

}  // namespace events::data_types
