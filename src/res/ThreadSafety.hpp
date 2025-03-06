#pragma once
#include<mutex>
class ThreadSafety_BaseC {
private:
    struct _ThreadSafetyC {
        struct DataS {
            //why not just use recursive_mutex?
            //becouse when some event happens it needs to unlock the mutex for other thread to lock it on event
            //but with recursive mutex i will have to record how much times it was locked and etc, much easier
            //to use normal mutex
            mutable std::mutex Mutex;
            std::condition_variable CV;
            bool InstanceAlreadyDestructed = false;
        };
        std::shared_ptr<DataS> Data;

        //used for more compact code about thread safety
        class LockC {
            std::shared_ptr<DataS> Ptr;
            bool FirstLock = false;
        public:
            LockC(ThreadSafety_BaseC const* inst) :Ptr(inst->ThreadSafety.Data) {
                if (inst->ThreadSafety.LastLockedThread != std::this_thread::get_id()) {
                    FirstLock = true;
                    Ptr->Mutex.lock();
                    inst->ThreadSafety.LastLockedThread = std::this_thread::get_id();
                }
            }
            ~LockC() { if(FirstLock) Ptr->Mutex.unlock(); }
            DataS* operator->() { return Ptr.get(); }
            template<typename LambdaT> void Wait(LambdaT&& lamb) {
                std::unique_lock ul(Ptr->Mutex, std::defer_lock); Ptr->CV.wait(ul, lamb); ul.release();
            }
            operator bool() { return !Ptr->InstanceAlreadyDestructed; }
        };
        mutable std::thread::id LastLockedThread;

        _ThreadSafetyC() :Data(new DataS) {}
        //you need to lock mutex in some upper destructor
        ~_ThreadSafetyC() {
            Data->InstanceAlreadyDestructed = true;
            Data->CV.notify_all();
            Data->Mutex.unlock();
        }
    };
protected:
    using ThreadLockC = _ThreadSafetyC::LockC;
    _ThreadSafetyC ThreadSafety;
};