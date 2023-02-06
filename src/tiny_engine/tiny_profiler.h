#pragma once
 
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>

// Author(s): The Cherno and davechurchill with some modifications by me
// https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e

// chrome tracing event format
// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview#

// HOW TO USE:
// Enable profiling -> run game -> close game -> open google chrome -> enter chrome://tracing into
// URL -> drag profiling_results.json onto that window

#define PROFILING 0
#if PROFILING
    #define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#else
    #define PROFILE_SCOPE(name)
#endif
#define PROFILE_FUNCTION()  PROFILE_SCOPE(__FUNCTION__)
 
struct ProfileResult
{
    const std::string name;
    long long start, end;
    uint32_t threadID;
};
 
class Profiler
{
    std::string     m_sessionName   = "None";
    std::ofstream   m_outputStream;
    int             m_profileCount  = 0;
    std::mutex      m_lock;
    bool            m_activeSession = false;
 
    Profiler() { }
 
public:
 
    static Profiler& Instance()
    {
        static Profiler instance;
        return instance;
    }
 
    ~Profiler()
    {
        endSession();
    }
 
    void beginSession(const std::string& name, const std::string& filepath = "profiler_results.json")
    {
        #if PROFILING
        if (m_activeSession) { endSession(); }
        m_activeSession = true;
        m_outputStream.open(filepath);
        writeHeader();
        m_sessionName = name;
        #endif
    }
 
    void endSession()
    {
        #if PROFILING
        if (!m_activeSession) { return; }
        m_activeSession = false;
        writeFooter();
        m_outputStream.close();
        m_profileCount = 0;
        #endif
    }
 
    void writeCompleteProfile(const ProfileResult& result)
    {
        std::lock_guard<std::mutex> lock(m_lock);
 
        if (m_profileCount++ > 0) { m_outputStream << ","; }
 
        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');
 
        m_outputStream << "{";
        m_outputStream << "\"cat\":\"function\",";
        m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
        m_outputStream << "\"name\":\"" << name << "\",";
        m_outputStream << "\"ph\":\"X\","; // "X" denotes a "complete" profile... I.E. we specify a "ts" (timestamp) and duration, and thats the total time for this function
        m_outputStream << "\"pid\":0,";
        m_outputStream << "\"tid\":" << result.threadID << ",";
        m_outputStream << "\"ts\":" << result.start;
        m_outputStream << "}";
    }
    void writeBeginProfile(const ProfileResult& result)
    {
        std::lock_guard<std::mutex> lock(m_lock);
 
        if (m_profileCount++ > 0) { m_outputStream << ","; }
 
        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');
 
        m_outputStream << "{";
        m_outputStream << "\"cat\":\"function\",";
        //m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
        m_outputStream << "\"name\":\"" << name << "\",";
        m_outputStream << "\"ph\":\"B\",";
        m_outputStream << "\"pid\":0,";
        m_outputStream << "\"tid\":" << result.threadID << ",";
        m_outputStream << "\"ts\":" << result.start;
        m_outputStream << "}";
    }
    void writeEndProfile(const ProfileResult& result)
    {
        std::lock_guard<std::mutex> lock(m_lock);
 
        if (m_profileCount++ > 0) { m_outputStream << ","; }
 
        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');
 
        m_outputStream << "{";
        m_outputStream << "\"cat\":\"function\",";
        //m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
        m_outputStream << "\"name\":\"" << name << "\",";
        m_outputStream << "\"ph\":\"E\",";
        m_outputStream << "\"pid\":0,";
        m_outputStream << "\"tid\":" << result.threadID << ",";
        m_outputStream << "\"ts\":" << result.end;
        m_outputStream << "}";
    }
 
    void writeHeader()
    {
        m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
    }
 
    void writeFooter()
    {
        m_outputStream << "]}";
    }
};
 
class InstrumentationTimer
{
    ProfileResult m_result;
 
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
    bool m_stopped;
 
public:
 
    InstrumentationTimer(const std::string & name)
        : m_result({ name, 0, 0, 0 })
        , m_stopped(false)
    {
        start();
    }
 
    ~InstrumentationTimer()
    {
        if (!m_stopped) { stop(); }
    }
 
    void start() {
        m_startTimepoint = std::chrono::high_resolution_clock::now();
        m_result.start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
        m_result.threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        Profiler::Instance().writeBeginProfile(m_result);
    }
    void stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();
 
        //m_result.start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
        m_result.end   = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
        m_result.threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        //Profiler::Instance().writeCompleteProfile(m_result);
        Profiler::Instance().writeEndProfile(m_result);
 
        m_stopped = true;
    }
};