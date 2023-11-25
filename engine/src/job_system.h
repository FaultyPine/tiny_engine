#ifndef JOB_SYSTEM_H
#define JOB_SYSTEM_H

//#include "pch.h"
#include "tiny_defines.h"
#include "containers/mutex_queue.h"
#include <functional>


struct Job {
    std::function<void()> func;
    u32 id;
};

struct JobSystem {

    TAPI void Initialize();
    TAPI void Shutdown() { numThreads = 0; }
    TAPI u32 Execute(const std::function<void()>& job);
    TAPI void ExecuteOnMainThread(const std::function<void()>& job);
    TAPI void WaitOnJob(u32 id);

    static JobSystem& Instance() {
        static JobSystem js;
        return js;
    }

    // called from engine main loop at the end of a frame
    void FlushMainThreadJobs();

private:

    #define MAX_JOBS 256
    MutexQueue<Job, MAX_JOBS> jobPool = {};
    MutexQueue<Job, MAX_JOBS> mainThreadJobPool = {};
    u32 currentJobID = 0; // provides unique identifier for every job
    std::vector<std::vector<u32>> inProgressJobs = {};
    u32 numThreads = 1;
};


#endif