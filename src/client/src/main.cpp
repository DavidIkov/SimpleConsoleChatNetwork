#include<iostream>
#include"AsioInclude.hpp"
#include"asio/ts/buffer.hpp"
#include"asio/ts/internet.hpp"
#include<thread>
#include<chrono>
#include<queue>
#include<condition_variable>
#include<functional>
#include"RemoveArrayPointer.hpp"
#include"Client.hpp"

char ReadingBuffer[1024];
asio::io_context CurContext;
ClientC Client(CurContext, std::string_view(ReadingBuffer, 1024));

#include"ConsoleCommands.hpp"

class MultithreadedOutputC {
public:
    std::condition_variable UpdateOutputCV;
    struct OutputtingProcessS {
        //position of last output for a process, relative to bottom empty line after all of output
        int PosX = 0, PosY = 0;
        std::string Buffer;
        std::mutex BufferMutex;
        bool operator==(const OutputtingProcessS& proc) const noexcept { return &proc == this; }
        template<typename T> OutputtingProcessS& operator<<(T&& v) { std::lock_guard lg(BufferMutex); Buffer += v; return *this; }
    };
protected:
    std::mutex OutputMutex;
    std::list<OutputtingProcessS> OutputtingProcesses;
    //position relative to bottom empty line after all of output
    int CursorPosX = 0, CursorPosY = 0;

    bool StopOutputThread = false;
    std::thread OutputThread = std::thread([&] {
        while (true) {
            std::unique_lock ul(OutputMutex);
            UpdateOutputCV.wait(ul, [&]()->bool {
                if (StopOutputThread) return true;
                for (auto& it : OutputtingProcesses) if (!it.Buffer.empty()) return true;
                return false;
                });
            if (StopOutputThread) return;
            unsigned consoleSizeX = ConsoleManagerNS::gConsoleSize().Colums;
            for (auto procIter = OutputtingProcesses.begin();procIter != OutputtingProcesses.end();procIter++)
                if (!procIter->Buffer.empty()) {
                    auto& proc = *procIter;
                    std::lock_guard lg(proc.BufferMutex);
                    {//move cursor
                        if (CursorPosX > proc.PosX) std::cout << "\x1b[" << std::to_string(CursorPosX - proc.PosX) << 'D';
                        else if (CursorPosX < proc.PosX) std::cout << "\x1b[" << std::to_string(proc.PosX - CursorPosX) << 'C';
                        if (CursorPosY > proc.PosY) std::cout << "\x1b[" << std::to_string(CursorPosY - proc.PosY) << 'B';
                        else if (CursorPosY < proc.PosY) std::cout << "\x1b[" << std::to_string(proc.PosY - CursorPosY) << 'A';
                    }
                    for (size_t i = 0;i < proc.Buffer.size();i++) {
                        if (proc.PosX == 0) {
                            //fully working way to create a new page(\x1b[L) considering all windows terminal fuckery
                            if (proc.PosY != 0) std::cout << "\x1b[" << std::to_string(proc.PosY) << 'B';
                            std::cout << "\n\x1b[1A";//creating new line so terminal will not eat bottom line
                            if (proc.PosY != 0) std::cout << "\x1b[" << std::to_string(proc.PosY) << 'A';
                            std::cout << "\x1b[L";
                            for (auto procIter2 = procIter;procIter2 != OutputtingProcesses.end();procIter2++) procIter2->PosY++;
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
                    CursorPosX = proc.PosX, CursorPosY = proc.PosY;
                    proc.Buffer.clear();
                }
        }
        });
public:
    OutputtingProcessS& CreateOutputtingProcess() { return OutputtingProcesses.emplace_back(); }
    void RemoveOutputtingProcess(OutputtingProcessS& proc) { proc.BufferMutex.lock(); OutputtingProcesses.remove(proc); }
    ~MultithreadedOutputC() {
        StopOutputThread = true; UpdateOutputCV.notify_all();
        OutputThread.join();
    }
};

int main(int argc, char** argv) {
    ConsoleManagerNS::EnableSettings();
    using namespace std::chrono_literals;
    MultithreadedOutputC MOUT;
    std::thread th1([&] {
        auto& proc1 = MOUT.CreateOutputtingProcess();
        while (true) {
            //std::this_thread::sleep_for(std::chrono::seconds(2));
            std::this_thread::sleep_for(std::chrono::milliseconds((long long)((std::rand() & 0xf) / (float)0xf * 2123.f)));
            proc1 << "thread 1 output!";
            MOUT.UpdateOutputCV.notify_all();

        }
        });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread th2([&] {
        auto& proc = MOUT.CreateOutputtingProcess();
        while (true) {
            //std::this_thread::sleep_for(std::chrono::seconds(2));
            std::this_thread::sleep_for(std::chrono::milliseconds((long long)((std::rand() & 0xf) / (float)0xf * 2123.f)));
            proc << "thread 2 output!";
            MOUT.UpdateOutputCV.notify_all();
        }
        });
    th1.join(), th2.join();
    asio::error_code CurErrorCode;
    asio::io_context::work IdleWork(CurContext);
    std::thread ContextThread([&] {CurContext.run();});

    std::thread th = ConsoleCommandsNS::InitializeConsoleReadingThread();

    th.join();
    CurContext.stop();
    IdleWork.~work();
    ContextThread.join();
    return 0;
}

