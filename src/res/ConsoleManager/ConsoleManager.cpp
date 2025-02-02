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

void ConsoleManagerNS::Initialize() {
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
char ConsoleManagerNS::InputNS::ReadChar() {
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
void ConsoleManagerNS::InputNS::ReadChars(size_t* amount, char* str) {
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
unsigned  ConsoleManagerNS::gConsoleColumnsAmount() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#elif defined __LINUX__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return { w.ws_col, w.ws_row };
#else
#error unknown platform
#endif
}
std::thread ConsoleManagerNS::OutputNS::_OutputThread = std::thread([] {
    while (true) {
        std::unique_lock ul(_OutputMutex);
        UpdateOutputCV.wait(ul, [&]()->bool {
            if (_StopOutputThread) return true;
            for (auto& it : _OutputtingProcesses) if (!it.Buffer.empty()) return true;
            return false;
            });
        if (_StopOutputThread) return;
        unsigned consoleSizeX = gConsoleColumnsAmount();
        for (auto procIter = _OutputtingProcesses.begin();procIter != _OutputtingProcesses.end();procIter++)
            if (!procIter->Buffer.empty()) {
                auto& proc = *procIter;
                std::lock_guard lg(proc.ProcMutex);
                proc.Outputting = true;
                {//move cursor
                    if (_CursorPosX > proc.PosX) std::cout << "\x1b[" << std::to_string(_CursorPosX - proc.PosX) << 'D';
                    else if (_CursorPosX < proc.PosX) std::cout << "\x1b[" << std::to_string(proc.PosX - _CursorPosX) << 'C';
                    if (_CursorPosY > proc.PosY) std::cout << "\x1b[" << std::to_string(_CursorPosY - proc.PosY) << 'B';
                    else if (_CursorPosY < proc.PosY) std::cout << "\x1b[" << std::to_string(proc.PosY - _CursorPosY) << 'A';
                }
                for (size_t i = 0;i < proc.Buffer.size();i++) {
                    if (proc.PosX == 0) {
                        //fully working way to create a new page(\x1b[L) considering all windows terminal fuckery
                        if (proc.PosY != 0) std::cout << "\x1b[" << std::to_string(proc.PosY) << 'B';
                        std::cout << "\n\x1b[1A";//creating new line so terminal will not eat bottom line
                        if (proc.PosY != 0) std::cout << "\x1b[" << std::to_string(proc.PosY) << 'A';
                        std::cout << "\x1b[L";
                        for (auto procIter2 = procIter;procIter2 != _OutputtingProcesses.end();procIter2++)
                            //no need for lock since positions are accessed only by this thread
                            procIter2->PosY++;
                    }
                    char ch = proc.Buffer[i];
                    if (ch < 32 || ch > 126) ch = '?';
                    std::cout << ch;
                    if (++proc.PosX == consoleSizeX) {
                        proc.PosY--;
                        proc.PosX = 0;
                        std::cout << "\n\r";
                    }
                }
                std::cout << std::flush;
                _CursorPosX = proc.PosX, _CursorPosY = proc.PosY;
                proc.Buffer.clear();
                proc.Outputting = false;
                proc.OutputEndedCV.notify_all();
            }
    }
    });

auto ConsoleManagerNS::OutputNS::CreateOutputtingProcess()->OutputtingProcessS& { return _OutputtingProcesses.emplace_back(); }
void ConsoleManagerNS::OutputNS::RemoveOutputtingProcess(OutputtingProcessS& proc) {
    std::unique_lock ul(proc.ProcMutex);
    if (!proc.Buffer.empty()) {
        //looks like calling remove happened too quickly, before outputting thread was able to actually output
        //calling flush cuz what if someone forgot to flush before calling this function, and if outputting
        //thread is not outputting at the moment fo aclling this function, this function will just freeze until
        //someones other flush
        proc << OutputtingProcessS::FlushOutput;
        //now just waiting for buffer to finish outputting
        proc.OutputEndedCV.wait(ul, [&]()->bool {return proc.Buffer.empty() && !proc.Outputting;});
    }
    _OutputtingProcesses.remove(proc);
}
void ConsoleManagerNS::OutputNS::Terminate() {
    _StopOutputThread = true; UpdateOutputCV.notify_all();
    _OutputThread.join();
}