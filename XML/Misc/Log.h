//////////////////////////////////////////////////////////////////////////
// Log.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef Log_INCLUDED
#define Log_INCLUDED

#include <fstream>
#include <vector>
#include <list>
#include "CriticalSection.h"
#include "FileSpec.h"
#include <map>

class CriticalSection;

//////////////////////////////////////////////////////////////////////////
#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)

//////////////////////////////////////////////////////////////////////////
// Base class to write log
class LogWriter
{
public:
    LogWriter(int id = -1);
    virtual ~LogWriter();

    virtual bool Write(const std::string& msg) = 0;

    int GetID() const;
    void SetID(int id);

protected:
    int mID;
};

//////////////////////////////////////////////////////////////////////////
// Write log to stdout
class OutputLogWriter: public LogWriter
{
public:
    OutputLogWriter(int id = 1);
    virtual ~OutputLogWriter();

    bool Write(const std::string& msg);
};

//////////////////////////////////////////////////////////////////////////
// Write log to Visual Studio Debug Output
class DebugOutputLogWriter: public LogWriter
{
public:
    DebugOutputLogWriter(int id = 2);
    virtual ~DebugOutputLogWriter();

    bool Write(const std::string& msg);
};

//////////////////////////////////////////////////////////////////////////
// Write log to file
class FileLogWriter: public LogWriter
{
public:
    FileLogWriter(int id = 3, const std::string& path = "");
    virtual ~FileLogWriter();

    bool Write(const std::string& msg);

    std::string GetFilePath() const;
    void SetFilePath(const std::string& path);
    void SetMaxSize(UInt64 maxSize);

private:
    bool InitLogFile();
    void CloseLogFile();

private:
    std::string mFilePath;
    FileSpec mLogSpec;
    UInt64 mMaxSize;
};

//////////////////////////////////////////////////////////////////////////
// Write log to log server
class NetLogWriter: public LogWriter
{
public:
    NetLogWriter(int sendInterval = 2, int maxIdleCount = 10, int maxFailedCount = 1000);
    bool Write(const std::string& msg);

    void SetSendInterval(int sendInterval);
    void SetMaxIdleCount(int maxIdleCount);
    void SetMaxFailedCount(int maxFailedCount);

private:
    void LogSendLoop();
    virtual bool Send(const std::list<std::string> &logs) = 0;
    static void *LogTaskFunc(void *writerObj);

private:
    // Use 2 log containers to buffer the log.
    // Writing-log buffer: The live log will be written here
    // Sending-log buffer: A task will send logs here to log server, periodically.
    // The two buffers will be switched when it's time to sending logs.
    // Use another log container to store the failed-sending logs.
    // Will try to send out those failed logs in next time.
    std::list<std::string> mLogBuf[3];
    int mWritingIdx;
    int mSendingIdx;
    int mFailedIdx;

    // The log task will do sending periodically (seconds).
    int mSendInterval;
    // The log task will exit when being long time idle
    int mMaxIdleCount;
    // Only keep up to 1000 (by default) failed-sending log
    int mMaxFailedCount;

    // Thread id of task
    unsigned long int mTaskThread;

    CriticalSection mCriticalSection;
};

//////////////////////////////////////////////////////////////////////////
enum LogLevel
{
    LogFatal = 1, LogError, LogWarning, LogInfo, LogDebug, LogTrace
};

//////////////////////////////////////////////////////////////////////////
class Log
{
public:
    Log(int redundancyFilterInterval = 5); // 5 seconds by default
    ~Log();

    static Log& Instance();

    // Log will delete the memory of LogWriter
    bool AddLogWriter(LogWriter* writter);
    bool RemoveLogWriter(LogWriter* writter);

    void SetOutputTime(bool output);
    void SetMaxLogLevel(LogLevel maxLevel);

    void Output(LogLevel level, const char* formatMsg, ...);
    void Output(LogLevel level, const char* functionName,
            unsigned int lineNumber, const char* formatMsg, ...);

    static void InitDefaultLogs(const std::string& logPath);

private:
    std::string LogLevelToString(LogLevel level) const;
    bool RedundancyFilter(std::string& msg);

private:
    std::vector<LogWriter*> mLogWriters;
    bool mOutputTime;
    // LogLevel under this will be ignored
    LogLevel mMaxLevel;
    CriticalSection mCriticalSection;

    // Redundancy Filter
    // Only send one log, if there are so many the same logs happened, within a specific interval.
    //
    // Redundancy Filter Interval
    int mRedundancyFilterInterval; // second
    // Sent log message status
    std::map<std::string, std::pair<long long, int> > mMessageSent;
};

#define LOG(LEVEL, ...) Log::Instance().Output(LEVEL, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG2(LEVEL, ...) Log::Instance().Output(LEVEL, __VA_ARGS__)

#endif // Log_INCLUDED
