#pragma once

#include <thread>

// set name of this thread
void SetThreadName(const char* threadName);
// set name of specified thread
void SetThreadName(std::thread* thread, const char* threadName);




