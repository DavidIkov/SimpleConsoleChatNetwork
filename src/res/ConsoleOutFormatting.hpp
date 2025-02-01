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
//makes so all characters will be normal characters, that are just 1 column sized in console,
//if character is not basic ascii character(excluding special symbols) it will be replaced with '?'
inline void MakeOneColumnSizedText(std::string& str) {
    for (size_t i = 0;i < str.size();i++)
        if (str[i] < 32 || str[i]>126) str[i] = '?';
}