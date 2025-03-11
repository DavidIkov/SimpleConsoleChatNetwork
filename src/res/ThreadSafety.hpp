#pragma once
#include<mutex>
#include<condition_variable>
class ThreadSafety_BaseC {
private:
    class _ThreadSafetyC {
    private:
        struct DataS {
            mutable std::mutex Mutex;
            std::condition_variable CV;
            bool InstanceAlreadyDestructed = false;
        };
        std::shared_ptr<DataS> Data;
    public:
        std::condition_variable& gCV() { return Data->CV; }
        void LockThread() const {
            if (LastLockedThread != std::this_thread::get_id() || LockDepth == 0) {
                Data->Mutex.lock();
                LastLockedThread = std::this_thread::get_id();
            }
            LockDepth++;
        }

        //used for more compact code about thread safety
        class LockC {
            ThreadSafety_BaseC const* Inst;
        public:
            LockC(ThreadSafety_BaseC const* inst) :Inst(inst) {
                inst->ThreadSafety.LockThread();
            }
            ~LockC() {
                Inst->ThreadSafety.LockDepth--;
                if (Inst->ThreadSafety.LockDepth == 0) Inst->ThreadSafety.Data->Mutex.unlock();
            }
            DataS* operator->() { return Inst->ThreadSafety.Data.get(); }
            template<typename LambdaT> void Wait(LambdaT&& lamb) {
                std::unique_lock ul(Inst->ThreadSafety.Data->Mutex, std::adopt_lock);
                Inst->ThreadSafety.LockDepth--;
                Inst->ThreadSafety.Data->CV.wait(ul, lamb); ul.release();
                Inst->ThreadSafety.LastLockedThread = std::this_thread::get_id();
                Inst->ThreadSafety.LockDepth++;
            }
            operator bool() { return !Inst->ThreadSafety.Data->InstanceAlreadyDestructed; }
        };
    private:
        mutable size_t LockDepth = 0;
        mutable std::thread::id LastLockedThread;
    public:
        _ThreadSafetyC() :Data(new DataS) {}
        //you need to lock mutex in some upper destructor with LockThread function
        ~_ThreadSafetyC() {
            Data->InstanceAlreadyDestructed = true;
            Data->CV.notify_all();
            LockDepth--;
            if (LockDepth == 0) Data->Mutex.unlock();
        }
    };
protected:
    using ThreadLockC = _ThreadSafetyC::LockC;
    _ThreadSafetyC ThreadSafety;
};