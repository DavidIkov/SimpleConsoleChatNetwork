#pragma once
#include<iostream>
template<typename elem, typename traits>
std::basic_ostream<elem, traits>& PutBeforeLastString(std::basic_ostream<elem, traits>& ostr) {
    return ostr << "\x1b[s\x1b[L\r";
}
template<typename elem, typename traits>
std::basic_ostream<elem, traits>& RestoreCursorPos(std::basic_ostream<elem, traits>& ostr) {
    return ostr << "\x1b[u\x1b[1B";
}