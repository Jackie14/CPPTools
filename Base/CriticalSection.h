//////////////////////////////////////////////////////////////////////////
// CriticalSection.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef CriticalSection_INCLUDED
#define CriticalSection_INCLUDED

#include <pthread.h>
#define LOCK pthread_mutex_t


//////////////////////////////////////////////////////////////////////////
class CriticalSection
{
public:
    CriticalSection();
    ~CriticalSection();

    void Lock();
    void Unlock();

private:
    void Init();
    void Clean();

private:
    LOCK mLock;
};

//////////////////////////////////////////////////////////////////////////
class AutoCriticalSection
{
public:
    AutoCriticalSection(CriticalSection *lock);
    ~AutoCriticalSection();

private:
    CriticalSection *mLock;
};

#endif // CriticalSection_INCLUDED
