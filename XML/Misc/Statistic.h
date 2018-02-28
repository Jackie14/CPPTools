/*
 * Statistic.h
 *
 */

#ifndef STATISTIC_H_
#define STATISTIC_H_

#include <map>
#include <string>
#include "CriticalSection.h"

class Statistic
{
public:
    Statistic(const std::string &filePath = "");

    void Increase(const std::string &key, long increase = 1);
    long Value(const std::string &key) const;
    void Reset(const std::string &key = "");
    std::string ToJsonString() const;
    bool LoadFromJsonString(const std::string &jsonStr);

    bool Load();
    bool SyncSave();
    bool AsyncSave();

private:
    bool LoadImpl(std::string &jsonStr);
    bool SaveImpl(std::map<std::string, long> &statistic) const;
    void AsyncSaveLoop();
    static void *AsyncSaveTaskFunc(void *objPtr);

    static bool ParseFromJson(const std::string& jsonStr, std::map<std::string, long>& statistic);
    static bool ComposeIntoJson(const std::map<std::string, long>& statistic, std::string& jsonStr);

    bool IsPersisEnabled();

private:
    std::map<std::string, long> mStatistic;

    // Need to save (there are changes in memory statistic)
    bool mNeedSave;
    // Thread id of async save task
    unsigned long int mSaveTaskThread;
    // File path of statistic file
    std::string mFilePath;
    // Lock for statistic file
    mutable CriticalSection mFileLock;

    // Lock for statistic
    mutable CriticalSection mCriticalSection;
};


#endif /* STATISTIC_H_ */
