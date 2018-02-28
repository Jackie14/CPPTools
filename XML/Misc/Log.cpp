//////////////////////////////////////////////////////////////////////////
// Log.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <pthread.h>
#include "stdarg.h"
#include <time.h>
#include "Log.h"
#include <ctime>
#include <string.h>
#include "Timestamp.h"
#include "DateTimeFormatter.h"

//////////////////////////////////////////////////////////////////////////
#if defined _WIN32
#define LINEEND ("\r\n")
#elif defined (_MACOSX)
#define LINEEND "\r"
#else
#define LINEEND "\n"
#endif

//////////////////////////////////////////////////////////////////////////
LogWriter::LogWriter(int id)
{
    mID = id;
}

LogWriter::~LogWriter()
{
}

int LogWriter::GetID() const
{
    return mID;
}

void LogWriter::SetID(int id)
{
    mID = id;
}

//////////////////////////////////////////////////////////////////////////
OutputLogWriter::OutputLogWriter(int id)
{
    mID = id;
}

OutputLogWriter::~OutputLogWriter()
{
}

bool OutputLogWriter::Write(const std::string& msg)
{
    if (msg.size() <= 0)
    {
        return false;
    }

    std::cout << msg.c_str() << std::flush;
    return true;
}

//////////////////////////////////////////////////////////////////////////
DebugOutputLogWriter::DebugOutputLogWriter(int id)
{
    mID = id;
}

DebugOutputLogWriter::~DebugOutputLogWriter()
{
}

bool DebugOutputLogWriter::Write(const std::string& msg)
{
    if (msg.size() <= 0)
    {
        return false;
    }

    std::cout << msg.c_str() << std::flush;
    return true;
}

//////////////////////////////////////////////////////////////////////////
FileLogWriter::FileLogWriter(int id, const std::string& path)
{
    mID = id;
    mFilePath = path;
    // 10 MB
    mMaxSize = 10 * 1048576; //1024 * 1024;

    InitLogFile();
}

FileLogWriter::~FileLogWriter()
{
    CloseLogFile();
}

bool FileLogWriter::InitLogFile()
{
    CloseLogFile();

    if (mFilePath.size() <= 0)
    {
        return false;
    }

    mLogSpec = mFilePath.c_str();
    bool result = mLogSpec.Open();

    return result;
}

void FileLogWriter::CloseLogFile()
{
    mLogSpec.Close();
}

bool FileLogWriter::Write(const std::string& msg)
{
    if (msg.size() <= 0)
    {
        return false;
    }

    // If size is too large, create a new one
    if (!mLogSpec.Exists())
    {
        mLogSpec.Create();
    }
    else if (mLogSpec.GetSize() >= mMaxSize)
    {
        mLogSpec.Recreate();
    }

    bool result = mLogSpec.Open();
    if (!result)
    {
        return false;
    }

    mLogSpec.Seek(0, 2); // Seek to end
    mLogSpec.Write(msg.c_str(), msg.length());
    mLogSpec.Close();
    return true;
}

std::string FileLogWriter::GetFilePath() const
{
    return mFilePath;
}

void FileLogWriter::SetFilePath(const std::string& path)
{
    mFilePath = path;
    InitLogFile();
}

void FileLogWriter::SetMaxSize(UInt64 maxSize)
{
    mMaxSize = maxSize;
}

//////////////////////////////////////////////////////////////////////////
NetLogWriter::NetLogWriter(int sendInterval,
        int maxIdleCount, int maxFailedCount)
{
    // Init the indexes of those log containers
    mWritingIdx = 0;
    mSendingIdx = 1;
    mFailedIdx = 2;

    mSendInterval = sendInterval;
    mMaxIdleCount = maxIdleCount;
    mMaxFailedCount = maxFailedCount;

    mTaskThread = 0;
}

bool NetLogWriter::Write(const std::string& msg)
{
    AutoCriticalSection autoLock(&mCriticalSection);

    // Write the log into pending buffer
    mLogBuf[mWritingIdx].push_back(msg);

    // Start log task, if need
    if (mTaskThread == 0)
    {
        if (pthread_create(&mTaskThread, NULL, LogTaskFunc, (void *)this) != 0)
        {
            return false;
        }
        pthread_detach(mTaskThread);
    }

    return true;
}

void NetLogWriter::SetSendInterval(int sendInterval)
{
    mSendInterval = sendInterval;
}

void NetLogWriter::SetMaxIdleCount(int maxIdleCount)
{
    mMaxIdleCount = maxIdleCount;
}

void NetLogWriter::SetMaxFailedCount(int maxFailedCount)
{
    mMaxFailedCount = maxFailedCount;
}

// 1) Periodically, sending logs within sending-buffer to log server.
// 2) Exit, when being long time idle.
void NetLogWriter::LogSendLoop()
{
    int idleCount = 0;

    while(true)
    {
        bool needSend = false;

        {// Use this {} to minimize the lock scope

            AutoCriticalSection autoLock(&mCriticalSection);

            if (!mLogBuf[mWritingIdx].empty())
            {
                // Reset idle count, since there are logs need to be sent
                idleCount = 0;

                // Switch log buffer (Writing <--> Sending)
                int temp = mWritingIdx;
                mWritingIdx = mSendingIdx;
                mSendingIdx = temp;

                needSend = true;
            }
            else if (!mLogBuf[mFailedIdx].empty())
            {
                // Also increase idle count, since there is only previous sending-failed logs.
                idleCount++;

                needSend = true;
            }
            else
            {
                // Increase idle count when there is not log to be sent
                idleCount++;

                if (idleCount >= mMaxIdleCount)
                {
                    // Exit when being long time idle
                    mTaskThread = 0;
                    return;
                }

                needSend = false;
            }
        }

        if (!needSend)
        {
            // Continue to next try, if nothing to send
            sleep(mSendInterval);
            continue;
        }

        // Move the previous sending-failed logs into sending buffer
        if (!mLogBuf[mFailedIdx].empty())
        {
            mLogBuf[mSendingIdx].insert(mLogBuf[mSendingIdx].begin(),
                    mLogBuf[mFailedIdx].begin(), mLogBuf[mFailedIdx].end());
            mLogBuf[mFailedIdx].clear();
        }

        // Send all logs within sending buffer to log server
        if (!Send(mLogBuf[mSendingIdx]))
        {
            // Store logs while sending failed, wait for next sending
            for (std::list<std::string>::const_iterator citer = mLogBuf[mSendingIdx].begin();
                    citer != mLogBuf[mSendingIdx].end() && (long)(mLogBuf[mFailedIdx].size()) < mMaxFailedCount;
                    citer++)
            {
                mLogBuf[mFailedIdx].push_back(*citer);
            }
        }

        // Clean the sending buffer, prepare for next sending.
        mLogBuf[mSendingIdx].clear();

        sleep(mSendInterval);
    }
}

void *NetLogWriter::LogTaskFunc(void *objPtr)
{
    if (objPtr)
    {
        ((NetLogWriter *)objPtr)->LogSendLoop();
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////
Log::Log(int redundancyFilterInterval)
{
    mOutputTime = true;
    mMaxLevel = LogDebug;
    mRedundancyFilterInterval = redundancyFilterInterval;
}

Log::~Log()
{
    // Clean
    for (std::vector<LogWriter*>::iterator iter = mLogWriters.begin();
            iter != mLogWriters.end(); ++iter)
    {
        delete *iter;
        (*iter) = NULL;
    }
}

Log& Log::Instance()
{
    static Log log;
    return log;
}

bool Log::AddLogWriter(LogWriter* writter)
{
    AutoCriticalSection autoLock(&mCriticalSection);

    if (writter == NULL)
    {
        return false;
    }

    mLogWriters.push_back(writter);
    return true;
}

bool Log::RemoveLogWriter(LogWriter* writter)
{
    AutoCriticalSection autoLock(&mCriticalSection);

    if (writter == NULL)
    {
        return false;
    }

    for (std::vector<LogWriter*>::iterator iter = mLogWriters.begin();
            iter != mLogWriters.end();)
    {
        LogWriter* temp = (*iter);
        if (temp && temp->GetID() == writter->GetID())
        {
            delete *iter;
            iter = mLogWriters.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    return true;
}

void Log::SetOutputTime(bool output)
{
    mOutputTime = output;
}

void Log::SetMaxLogLevel(LogLevel maxLevel)
{
    mMaxLevel = maxLevel;
}

#define BufferSize 4096
void Log::Output(LogLevel level, const char* formatMsg, ...)
{
    // Check configuration log level
    if (level > mMaxLevel)
    {
        return;
    }

    //////////////////////////////////////////////////////////////////////////
    // Format string
    char buffer[BufferSize];
    va_list args;
    va_start(args, formatMsg);
    vsnprintf(buffer, sizeof(buffer), formatMsg, args);
    va_end(args);
    //////////////////////////////////////////////////////////////////////////

    std::string logMsg = std::string(buffer);
    if (logMsg.size() <= 0)
    {
        return;
    }

    // Add extra info
    std::string finalMsg;

    // Add Level
    std::string levelStr = LogLevelToString(level);
    finalMsg += levelStr;

    // __FILE__, TOSTRING(__LINE__)

    // Add User Input Message
    finalMsg += logMsg;

    // Do log redundancy filter
    if (!RedundancyFilter(finalMsg))
    {
        return;
    }

    // Add Time
    if (mOutputTime)
    {
        Timestamp timeStamp;
        std::string timeStr = DateTimeFormatter::Format(timeStamp,
                DateTimeFormat::SORTABLE_FORMAT);

        if (!timeStr.empty())
        {
            finalMsg = timeStr + " " + finalMsg;
        }
    }

    // Add New Line
    finalMsg += LINEEND;

    AutoCriticalSection autoLock(&mCriticalSection);

    for (std::vector<LogWriter*>::iterator iter = mLogWriters.begin();
            iter != mLogWriters.end(); ++iter)
    {
        LogWriter* temp = (*iter);
        if (temp == NULL)
        {
            continue;
        }

        temp->Write(finalMsg);
    }
}

void Log::Output(LogLevel level, const char* functionName,
        unsigned int lineNumber, const char* formatMsg, ...)
{
    // Check configuration log level
    if (level > mMaxLevel)
    {
        return;
    }

    //////////////////////////////////////////////////////////////////////////
    // Format string
    char buffer[BufferSize];
    va_list args;
    va_start(args, formatMsg);
    vsnprintf(buffer, sizeof(buffer), formatMsg, args);
    va_end(args);
    //////////////////////////////////////////////////////////////////////////

    std::string logMsg = std::string(buffer);
    if (logMsg.size() <= 0)
    {
        return;
    }

    // Add extra info
    std::string finalMsg;

    // Add Level
    std::string levelStr = LogLevelToString(level);
    finalMsg += levelStr;

    // Add function and line
    if (functionName != NULL && strlen(functionName) > 0)
    {
        finalMsg += functionName;
        finalMsg += ": ";

        if (lineNumber > 0)
        {
            char lineBuffer[64];
            std::sprintf(lineBuffer, "%d", lineNumber);
            finalMsg += lineBuffer;
            finalMsg += ". ";
        }
    }

    // Add User Input Message
    finalMsg += logMsg;

    // Do log redundancy filter
    if (!RedundancyFilter(finalMsg))
    {
        return;
    }

    // Add Time
    if (mOutputTime)
    {
        Timestamp timeStamp;
        std::string timeStr = DateTimeFormatter::Format(timeStamp,
                DateTimeFormat::SORTABLE_FORMAT);

        if (!timeStr.empty())
        {
            finalMsg = timeStr + " "+ finalMsg;
        }
    }

    // Add New Line
    finalMsg += LINEEND;

    AutoCriticalSection autoLock(&mCriticalSection);

    for (std::vector<LogWriter*>::iterator iter = mLogWriters.begin();
            iter != mLogWriters.end(); ++iter)
    {
        LogWriter* temp = (*iter);
        if (temp == NULL)
        {
            continue;
        }

        temp->Write(finalMsg);
    }
}

// Return true, to send current log
// Return false, to abandon current log
bool Log::RedundancyFilter(std::string& msg)
{
    AutoCriticalSection autoLock(&mCriticalSection);

    long long curTime = Timestamp().GetEpochMicroseconds();

    ///////////////////////////////////////////////////
    // Send out the log message, when this kind of log message has never been sent
    ///////////////////////////////////////////////////
    std::map<std::string, std::pair<long long, int> >::iterator msgSent = mMessageSent.find(msg);
    if (msgSent == mMessageSent.end())
    {
        // Record this sent log message
        mMessageSent[msg].first = curTime;
        mMessageSent[msg].second = 0;
        return true;
    }

    std::pair<long long, int> &msgStatus = msgSent->second;
    long long &lastSentTime = msgStatus.first;
    int &lagCount = msgStatus.second;
    lagCount++; // Increase the count for current log

    ///////////////////////////////////////////////////
    // Send out the log message, when this kind of log message has not been sent within last <interval>.
    ///////////////////////////////////////////////////
    if (lastSentTime + mRedundancyFilterInterval * Timestamp::GetResolution() < curTime)
    {
        // Add duplicate count info in to the log message?
        if (lagCount > 1)
        {
            char countMsg[64];
            std::sprintf(countMsg, "[%d]", lagCount);
            msg += countMsg;
        }

        // Reset sent message status
        lastSentTime = curTime;
        lagCount = 0;

        return true;
    }

    /////////////////////////////////////////////////////
    // Do not send out the log message, since there is the same log message sent out with in last <interval>
    ////////////////////////////////////////////////////
    return false;
}

std::string Log::LogLevelToString(LogLevel level) const
{
    switch (level)
    {
    case LogInfo:
        return ("Info: ");
    case LogDebug:
        return ("Debug: ");
    case LogWarning:
        return ("Warning: ");
    case LogError:
        return ("ERROR: ");
    case LogFatal:
        return ("FATAL: ");
    case LogTrace:
        return ("Trace: ");
    default:
        return ("");
    }
}

void Log::InitDefaultLogs(const std::string& logPath)
{
    LogWriter* stdLog = new OutputLogWriter();
    LogWriter* fileLog = new FileLogWriter(2, logPath);
    LogWriter* debugLog = new DebugOutputLogWriter();
    Log::Instance().AddLogWriter(stdLog);
    Log::Instance().AddLogWriter(debugLog);
    Log::Instance().AddLogWriter(fileLog);
}
