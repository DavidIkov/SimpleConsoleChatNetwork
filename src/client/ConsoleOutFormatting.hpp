#pragma once
#include<iostream>
template<typename elem, typename traits>
std::basic_ostream<elem, traits>& PutBeforeLastString(std::basic_ostream<elem, traits>& ostr) {
    return ostr << "\x1b[L\r";
}