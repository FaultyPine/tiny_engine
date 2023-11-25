//#include "pch.h"
#include "job_system.h"
#include "tiny_log.h"

#include <thread>
#include <string>

void SetThreadName( std::thread* thread, const char* threadName);

void JobSystem::Initialize() {
    // get number of threads this system supports
    u32 numCores = std::thread::hardware_concurrency();
    // hardware_concurrency may return 0 if it can't query properly.. in that case just use 1
    // reasonable max concurrent threads... don't want to spin up 12 threads for this engine lol
    u32 threads = std::min(std::max(1u, numCores), 3u); 
    this->numThreads = threads;
    inProgressJobs = std::vector<std::vector<u32>>(numThreads);
    LOG_INFO("[JOBS] Spinning up %i job threads", numThreads);

    for (u32 threadID = 0; threadID < this->numThreads; threadID++) {
        std::thread worker([this, threadID]{

            Job job;
            std::vector<u32>& inProgressJobsForThread = inProgressJobs.at(threadID);
            while (numThreads > 0) { // allows us to shut down all threads when program exits by just setting numthreads to 0
                if (jobPool.pop_front(job)) {
                    inProgressJobsForThread.push_back(job.id);
                    job.func(); // execute the job
                    inProgressJobsForThread.erase(std::find(inProgressJobsForThread.begin(), inProgressJobsForThread.end(), job.id));
                    //std::cout << "[JOBS] Thread " << threadID << " finished job " << job.id << ""; 
                }
                // might be good in the future to put the thread to sleep when there's no more work
                // and when we enqueue a new job it'll wake the thread up. Use condition variables for this
            }
            //std::cout << "[JOBS] Shutting down job thread " << threadID << "";

        });
        SetThreadName(&worker, ("Job Thread " + std::to_string(threadID)).c_str());
        worker.detach(); // set it free
    }
}

void JobSystem::ExecuteOnMainThread(const std::function<void()>& job)
{
    Job jobWithID;
    jobWithID.func = job;
    jobWithID.id = currentJobID++;
    mainThreadJobPool.push_back(jobWithID);
}

void JobSystem::FlushMainThreadJobs()
{
    mainThreadJobPool.lock.lock();
    u32 mainThreadJobs = mainThreadJobPool.size();
    for (u32 i = 0; i < mainThreadJobs; i++)
    {
        mainThreadJobPool.get(i).func();
    }
    mainThreadJobPool.lock.unlock();
    mainThreadJobPool.clear();
}

u32 JobSystem::Execute(const std::function<void()>& job) {
    Job jobWithID;
    jobWithID.func = job;
    // because our job id's are "sequential", we can rely on the fact that
    // if a thread reports that they finished a job with id X, jobs with id <= X 
    // that were on that same thread have also finished
    jobWithID.id = currentJobID++;
    jobPool.push_back(jobWithID); // TODO: this might fail if we've hit MAX_JOBS???
    return jobWithID.id;
}

void JobSystem::WaitOnJob(u32 id) {
    bool jobInProgress = true;
    // spinlocks until the job is done
    while (jobInProgress) { // TODO: configurable timeout
        // if the job id we want to wait on is not "in progress" we can assume it is finished
        // this relies on the fact that waiting on a job will always happen after the job has been
        // "dispatched" - and that you are waiting on the intended job id
        for (s32 threadIdx = 0; threadIdx < inProgressJobs.size(); threadIdx++) {
            const std::vector<u32>& inProgressJobsForThread = inProgressJobs.at(threadIdx);
            if (!std::count(inProgressJobsForThread.begin(), inProgressJobsForThread.end(), id)) {
                jobInProgress = false;
            }
        }
    }
}


// set thread name. TODO: seperate this into Thread engine module

#ifdef _WIN32
#include <windows.h>
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


void SetThreadName(uint32_t dwThreadID, const char* threadName)
{

  // DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
void SetThreadName( const char* threadName)
{
    SetThreadName(GetCurrentThreadId(),threadName);
}

void SetThreadName( std::thread* thread, const char* threadName)
{
    DWORD threadId = ::GetThreadId( static_cast<HANDLE>( thread->native_handle() ) );
    SetThreadName(threadId,threadName);
}

#elif defined(__linux__)
#include <sys/prctl.h>
void SetThreadName( const char* threadName)
{
  prctl(PR_SET_NAME,threadName,0,0,0);
}

#else
void SetThreadName(std::thread* thread, const char* threadName)
{
   auto handle = thread->native_handle();
   pthread_setname_np(handle,threadName);
}
#endif