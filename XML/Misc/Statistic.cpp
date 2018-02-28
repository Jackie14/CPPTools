/*
 * Statistic.cpp
 *
 */

#include <iostream>
#include <pthread.h>
#include "Statistic.h"
#include "JSONParser.h"
#include "Log.h"
#include <auto_ptr.h>

Statistic::Statistic(const std::string &filePath) :
    mNeedSave(false), mSaveTaskThread(0), mFilePath(filePath)
{
    Load();
}

void Statistic::Increase(const std::string &key, long increase)
{
    AutoCriticalSection autoLock(&mCriticalSection);
    mStatistic[key] += increase;

    AsyncSave();
}

long Statistic::Value(const std::string &key) const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    std::map<std::string, long>::const_iterator citer =
            mStatistic.find(key);
    if (citer == mStatistic.end())
    {
        return 0;
    }

    return citer->second;
}

void Statistic::Reset(const std::string &key)
{
    AutoCriticalSection autoLock(&mCriticalSection);
    if (key == "")
    {
        mStatistic.clear();
    }
    else
    {
        mStatistic.erase(key);
    }

    AsyncSave();
}

std::string Statistic::ToJsonString() const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    std::string str;
    ComposeIntoJson(mStatistic, str);
    return str;
}

bool Statistic::LoadFromJsonString(const std::string &jsonStr)
{
    AutoCriticalSection autoLock(&mCriticalSection);
    return ParseFromJson(jsonStr, mStatistic);
}

bool Statistic::Load()
{
    if (!IsPersisEnabled())
    {
        LOG(LogDebug, "Do not enable statistic persist");
        return true;
    }

    std::string jsonStr;
    if (!LoadImpl(jsonStr))
    {
        return false;
    }

    return LoadFromJsonString(jsonStr);
}

bool Statistic::SyncSave()
{
    if (!IsPersisEnabled())
    {
        LOG(LogDebug, "Do not enable statistic persist");
        return true;
    }

    // Copy out statistic data within the lock
    std::map<std::string, long> statisticCopy;
    {

        AutoCriticalSection autoLock(&mCriticalSection);
        statisticCopy = mStatistic;
    }

    return SaveImpl(statisticCopy);
}

// The statistic data will be saved before system normal shutdown.
// We also has a task to save statistic data every 10 minutes
// Include this task is intending to avoid data lost when encounter system abnormal stop.
bool Statistic::AsyncSave()
{
    if (!IsPersisEnabled())
    {
        LOG(LogTrace, "Do not enable statistic persist");
        return true;
    }

    // Notice task: "we need to do save"
    mNeedSave = true;

    // Start task, if there is no task doing save
    if (mSaveTaskThread == 0)
    {
        if (pthread_create(&mSaveTaskThread, NULL, AsyncSaveTaskFunc, (void *)this) != 0)
        {
            return false;
        }
        pthread_detach(mSaveTaskThread);
    }

    return true;
}

bool Statistic::LoadImpl(std::string &jsonStr)
{
    AutoCriticalSection autoLock(&mFileLock);

    std::ifstream *file = NULL;
    std::ifstream filePersist(mFilePath.c_str());
    std::ifstream fileBackup((mFilePath + ".backup").c_str());
    if (filePersist)
    {
        file = &filePersist;
    }
    else if (fileBackup)
    {
        LOG(LogDebug, "Statistic file %s can't be opened, use backup file", mFilePath.c_str());
        file = &fileBackup;
    }
    else
    {
        LOG(LogDebug, "Statistic backup file %s can't be opened too",(mFilePath + ".backup").c_str());
        return false;
    }

    jsonStr.clear();
    jsonStr.assign(std::istreambuf_iterator<char>(*file),
            std::istreambuf_iterator<char>());

    file->close();
    if (jsonStr.empty())
    {
        LOG(LogDebug, "statistic file is empty");
        return false;
    }

    LOG(LogDebug, "Get statistic file content: %s", jsonStr.c_str());

    return true;
}

bool Statistic::SaveImpl(std::map<std::string, long> &statistic) const
{
    // Compose the statistic into string
    std::string statisticStr;
    ComposeIntoJson(statistic,statisticStr);

    AutoCriticalSection autoLock(&mFileLock);

    // Backup the previous one
    FileSpec f(mFilePath);
    if (f.Exists())
    {
        f.CopyTo(mFilePath + ".backup");
        f.Remove(false);
    }

    // Write new data into file
    if (!f.Open(true))
    {
        LOG(LogDebug, "Statistic file %s can't be opened", f.GetPath().c_str());
        return false;
    }

    return f.Write(statisticStr.data(), statisticStr.length());
}

void Statistic::AsyncSaveLoop()
{
    int idleCount = 0;

    LOG(LogDebug, "Start statistic persist task");

    while(true)
    {
        if (mNeedSave)
        {
            idleCount = 0;

            // Copy out statistic within the lock
            std::map<std::string, long> statisticCopy;
            {
                AutoCriticalSection autoLock(&mCriticalSection);
                statisticCopy = mStatistic;
                mNeedSave = false;
            }

            LOG(LogDebug, "Do statistic save in persist task");
            SaveImpl(statisticCopy);
        }
        else
        {
            idleCount++;

            // Exit when being long time idle
            if (idleCount > 3)
            {
                LOG(LogDebug, "Stop statistic persist task, for being too long time idle");
                mSaveTaskThread = 0;
                return;
            }
        }

        sleep(600); // 10 minutes
    }
}

void *Statistic::AsyncSaveTaskFunc(void *objPtr)
{
    if (objPtr)
    {
        ((Statistic *)objPtr)->AsyncSaveLoop();
    }

    return NULL;
}

bool Statistic::ParseFromJson(const std::string& jsonStr, std::map<std::string, long>& statistic)
{
    std::auto_ptr<JSONValue> root(JSONParser::Parse(jsonStr.c_str()));

    LOG(LogDebug, "Start to parse statistic file");

    if (root.get() == NULL ||
            !root->IsObject())
    {
        LOG(LogDebug, "Failed to parse statistic string %s", jsonStr.c_str());
        return false;
    }

    JSONObject o = root->AsObject();
    for (JSONObject::const_iterator citer = o.begin();
            citer != o.end();
            citer++)
    {
        JSONValue *v = citer->second;
        if (!v || !v->IsDouble())
        {
            continue;
        }

        LOG(LogDebug, "Get statistic %s = %ld", citer->first.c_str(), (long)(v->AsDouble()));

        statistic[citer->first] = (long)(v->AsDouble());
    }

    return true;
}

bool Statistic::ComposeIntoJson(const std::map<std::string, long>& statistic, std::string& jsonStr)
{
    JSONObject o;
    for (std::map<std::string, long>::const_iterator citer = statistic.begin();
            citer != statistic.end(); citer++)
    {
        o[citer->first] = new JSONValue((double) citer->second);
    }
    JSONValue msg(o);

    jsonStr = msg.ToString();

    return true;
}

bool Statistic::IsPersisEnabled()
{
    if (mFilePath.empty())
    {
        return false;
    }

    return true;
}
