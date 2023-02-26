#include "pch.h"
#include "job_system.h"

#include <thread>

void JobSystem::Initialize() {
    // get number of threads this system supports
    u32 numCores = std::thread::hardware_concurrency();
    u32 threads = std::max(1u, numCores); // hardware_concurrency may return 0 if it can't query properly.. in that case just use 1
    this->numThreads = threads;
    inProgressJobs = std::vector<std::vector<u32>>(numThreads);
    std::cout << "[JOBS] Spinning up " << numThreads << " job threads\n";

    for (u32 threadID = 0; threadID < this->numThreads; threadID++) {
        std::thread worker([this, threadID]{

            Job job;
            std::vector<u32>& inProgressJobsForThread = inProgressJobs.at(threadID);
            while (numThreads > 0) { // allows us to shut down all threads when program exits by just setting numthreads to 0
                if (jobPool.pop_front(job)) {
                    inProgressJobsForThread.push_back(job.id);
                    job.func(); // execute the job
                    inProgressJobsForThread.erase(std::remove(inProgressJobsForThread.begin(), inProgressJobsForThread.end(), job.id));
                    //std::cout << "[JOBS] Thread " << threadID << " finished job " << job.id << "\n"; 
                }
                // might be good in the future to put the thread to sleep when there's no more work
                // and when we enqueue a new job it'll wake the thread up. Use condition variables for this
            }
            //std::cout << "[JOBS] Shutting down job thread " << threadID << "\n";

        });
        worker.detach(); // set it free
    }
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
    while (jobInProgress) {
        // if the job id we want to wait on is not "in progress" we can assume it is finished
        // this relies on the fact that waiting on a job will always happen after the job has been
        // "dispatched"
        for (s32 threadIdx = 0; threadIdx < inProgressJobs.size(); threadIdx++) {
            const std::vector<u32>& inProgressJobsForThread = inProgressJobs.at(threadIdx);
            if (!std::count(inProgressJobsForThread.begin(), inProgressJobsForThread.end(), id)) {
                jobInProgress = false;
            }
        }
    }
}