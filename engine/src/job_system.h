#ifndef JOB_SYSTEM_H
#define JOB_SYSTEM_H

//#include "pch.h"
#include "tiny_defines.h"
#include "containers/mutex_queue.h"
#include <functional>

/*
C:\Users\gclar\Desktop>job_parser.py
(With 8000 ninjas onscreen)
Without job system average scene tick time = 0.00471090243902439094
With job system average scene tick time = 0.00006366292134831459
Job system improved scene tick time with 8000 ninjas by 73.9976%
*/

struct Job {
    std::function<void()> func;
    u32 id;
};

struct JobSystem {

    TAPI void Initialize();
    TAPI void Shutdown() { numThreads = 0; }
    TAPI u32 Execute(const std::function<void()>& job);
    TAPI void WaitOnJob(u32 id);

    static JobSystem& Instance() {
        static JobSystem js;
        return js;
    }

private:

    #define MAX_JOBS 256
    MutexQueue<Job, MAX_JOBS> jobPool;
    u32 currentJobID = 0; // provides unique identifier for every job
    std::vector<std::vector<u32>> inProgressJobs = {};
    u32 numThreads = 1;
};


#endif