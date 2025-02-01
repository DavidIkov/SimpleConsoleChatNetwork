#include"ConsoleManager.hpp"
#include<string>
#include<iostream>

#ifdef _WIN32
#include"Windows.h"
#elif defined __LINUX__
#include"termios.h"
#else
#error unknown platform
#endif

void ConsoleManagerNS::EnableSettings() {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT) | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#elif defined __LINUX__
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#else
#error unknown platform
#endif
    std::setvbuf(stdout, nullptr, _IOFBF, 1024);//enable requirement to flush before anything appears in console
}
char ConsoleManagerNS::ReadChar() {
#ifdef _WIN32
    char rch;
    DWORD charsRead;
    ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &rch, 1, &charsRead, nullptr);
    return rch;
#elif defined __LINUX__
    char rch; read(STDIN_FILENO, &rch, 1);
    return rch;
#else
#error unknown platform
#endif
}
void ConsoleManagerNS::ReadChars(size_t* amount, char* str) {
#ifdef _WIN32
    DWORD charsRead;
    ReadConsole(GetStdHandle(STD_INPUT_HANDLE), str, *amount, &charsRead, nullptr);
    *amount = charsRead;
#elif defined __LINUX__
    *amount = read(STDIN_FILENO, str, *amount);
#else
#error unknown platform
#endif
   
}
ConsoleManagerNS::ConsoleSizeS ConsoleManagerNS::gConsoleSize() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return { csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1 };
#elif defined __LINUX__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return { w.ws_col, w.ws_row };
#else
#error unknown platform
#endif
}
