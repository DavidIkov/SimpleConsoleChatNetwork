#include"ConsoleFixedSizeInput.hpp"
#include<string>
bool ConsoleFixedSizeInputC::Exists = false;
#ifdef _WIN32
#include"Windows.h"
#elif defined __LINUX__
#include"termios.h"
#else
#error unknown platform
#endif

ConsoleFixedSizeInputC::ConsoleFixedSizeInputC() {
    if (Exists) return;
    Exists = true;
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
#elif defined __LINUX__
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#else
#error unknown platform
#endif
}
char ConsoleFixedSizeInputC::ReadChar() {
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
void ConsoleFixedSizeInputC::ReadChars(size_t* amount, char* str) {
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
