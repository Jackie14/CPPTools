//////////////////////////////////////////////////////////////////////////
// CriticalSection.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "CriticalSection.h"

//////////////////////////////////////////////////////////////////////////
CriticalSection::CriticalSection()
{
    Init();
}

CriticalSection::~CriticalSection()
{
    Clean();
}

//////////////////////////////////////////////////////////////////////////
// Platforms specific
void CriticalSection::Lock()
{
    pthread_mutex_lock(&mLock);
}

void CriticalSection::Unlock()
{
    pthread_mutex_unlock(&mLock);
}

void CriticalSection::Init()
{
    //mLock = PTHREAD_MUTEX_INITIALIZER; 
    pthread_mutex_init(&mLock, NULL);
}

void CriticalSection::Clean()
{
    pthread_mutex_destroy(&mLock);
}

//////////////////////////////////////////////////////////////////////////
// AutoCriticalSection
AutoCriticalSection::AutoCriticalSection(CriticalSection *lock)
{
    if (lock)
    {
        mLock = lock;
        mLock->Lock();
    }
}

AutoCriticalSection::~AutoCriticalSection()
{
    if (mLock)
    {
        mLock->Unlock();
    }
}
