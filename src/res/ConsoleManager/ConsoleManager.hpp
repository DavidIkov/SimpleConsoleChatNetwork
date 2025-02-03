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
    //this should be threaded as namespace, not a class
    class OutputNS {
        static std::condition_variable UpdateOutputCV;
        static void OutputThreadFunc();
    public:
        class OutputtingProcessC {
            friend OutputNS;
            friend void OutputNS::OutputThreadFunc();
        protected:
            //position of last output for a process, relative to bottom empty line after all of output
            int PosX = 0, PosY = 0;
            std::mutex ProcMutex;
            std::string Buffer;
            bool Outputting = false;
            std::condition_variable OutputEndedCV;
        public:
            bool operator==(const OutputtingProcessC& proc) const noexcept { return &proc == this; }// ik ik
            template<typename T> OutputtingProcessC& operator<<(T&& v) {
                std::lock_guard lg(ProcMutex);
                Buffer.operator+=(std::forward<T>(v));
                return *this;
            }
            template<> OutputtingProcessC& operator<<<void(&)(OutputtingProcessC&)>(void(&v)(OutputtingProcessC&)) {
                v(*this); return *this;
            }
            //text may appear before flushing happened. calling flush makes so text will appear as soon as possible
            static void FlushOutput(OutputtingProcessC& proc) {
                UpdateOutputCV.notify_all();
            }
            //dosent flush output, just new line
            static void NewLine(OutputtingProcessC& proc) {
                proc << "\n";
            }
            //calls NewLine and FlushOutput
            static void EndLine(OutputtingProcessC& proc) {
                proc << NewLine << FlushOutput;
            }
        }; friend OutputtingProcessC;
    private:
        static std::mutex OutputMutex;
        static std::list<OutputtingProcessC> OutputtingProcesses;
        //position relative to bottom empty line after all of output
        static int CursorPosX, CursorPosY;//0,0

        static bool StopOutputThread;//false

        static std::thread OutputThread;
    public:
        OutputNS() = delete;
        static OutputtingProcessC& CreateOutputtingProcess();
        static void RemoveOutputtingProcess(OutputtingProcessC& proc);
        static void Terminate();
        class OutputtingProcessWrapperC {
            OutputtingProcessC& Proc;
        public:
            OutputtingProcessWrapperC() :Proc(CreateOutputtingProcess()) {};
            ~OutputtingProcessWrapperC() { RemoveOutputtingProcess(Proc); }
            static constexpr decltype(OutputtingProcessC::FlushOutput)& FlushOutput = OutputtingProcessC::FlushOutput;
            static constexpr decltype(OutputtingProcessC::NewLine)& NewLine = OutputtingProcessC::NewLine;
            static constexpr decltype(OutputtingProcessC::EndLine)& EndLine = OutputtingProcessC::EndLine;
            template<typename T> OutputtingProcessWrapperC& operator<<(T&& v) {
                Proc.operator<<(std::forward<T>(v)); return *this;
            }
            operator OutputtingProcessC& () { return Proc; }
        };
    };
}
