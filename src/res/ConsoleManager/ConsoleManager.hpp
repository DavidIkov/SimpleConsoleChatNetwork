#pragma once

#include<mutex>
#include<list>

namespace ConsoleManagerNS {
    //disables output of input in console
    //enables reading by specified amount of characters internally(with os api)
    //std::cin is now may be broken, use ReadChar instead
    //and for output use functionality provided by OutputNS instead of std::cout
    //enables full support for most ansi escape codes(still depends on os and terminal)
    void Initialize();
    unsigned gConsoleColumnsAmount();
    namespace InputNS {
        //when enter is pressed, '\r' will be written
        char ReadChar();
        //when enter is pressed, '\r' will be written
        void ReadChars(size_t* amount, char* str);
    }
    //focuses on multithreaded output
    //todo fix bug with console resizing. offsets dont update
    namespace OutputNS {
        inline std::condition_variable UpdateOutputCV;
        struct OutputtingProcessS {
            //position of last output for a process, relative to bottom empty line after all of output
            int PosX = 0, PosY = 0;
            std::string Buffer;
            std::mutex BufferMutex;
            bool operator==(const OutputtingProcessS& proc) const noexcept { return &proc == this; }
            template<typename T> OutputtingProcessS& operator<<(T&& v) {
                std::lock_guard lg(BufferMutex);
                Buffer.operator+=(std::forward<T>(v));
                return *this;
            }
            template<> OutputtingProcessS& operator<<<void(*const&)()>(void(*const&v)()) {
                v(); return *this;
            }
            static void FlushOutput() {
                UpdateOutputCV.notify_all();
            }
        };
        inline std::mutex _OutputMutex;
        inline std::list<OutputtingProcessS> _OutputtingProcesses;
        //position relative to bottom empty line after all of output
        inline int _CursorPosX = 0, _CursorPosY = 0;

        inline bool _StopOutputThread = false;
        extern std::thread _OutputThread;
        OutputtingProcessS& CreateOutputtingProcess();
        void RemoveOutputtingProcess(OutputtingProcessS& proc);
        void Terminate();
        class OutputtingProcessWrapperC {
        private:
            OutputtingProcessS& proc;
        public:
            OutputtingProcessWrapperC() :proc(CreateOutputtingProcess()) {};
            ~OutputtingProcessWrapperC() { RemoveOutputtingProcess(proc); }
            static constexpr decltype(&OutputtingProcessS::FlushOutput) FlushOutput = &OutputtingProcessS::FlushOutput;
            template<typename T> OutputtingProcessWrapperC& operator<<(T&& v) {
                proc.operator<<(std::forward<T>(v)); return *this;
            }
        };
    }
}
