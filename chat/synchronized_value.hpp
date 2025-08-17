#pragma once
#include <mutex>

template <typename T>
class SynchronizedValue {
public:
    SynchronizedValue(std::mutex& mutex, T&& v)
        : lg_(mutex), v_(std::forward<T>(v)) {}

private:
    std::lock_guard<std::mutex> lg_;

public:
    // accessing this value when lifetime of SynchronizedValue is ended, can
    // lead to UB.
    T&& v_;
};
