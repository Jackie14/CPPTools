/*
 * RWLock.cpp
 *
 */

#include "RWLock.h"

RWLock::RWLock()
{
    pthread_rwlock_init(&mRwLock, NULL);
}

RWLock::~RWLock()
{
    pthread_rwlock_destroy(&mRwLock);
}

void RWLock::ReadLock()
{
    pthread_rwlock_rdlock(&mRwLock);
}

void RWLock::WriteLock()
{
    pthread_rwlock_wrlock(&mRwLock);
}

void RWLock::UnLock()
{
    pthread_rwlock_unlock(&mRwLock);
}

AutoReadLock::AutoReadLock(RWLock &rwLock) : mRwLock(rwLock)
{
    mRwLock.ReadLock();
}

AutoReadLock::~AutoReadLock()
{
    mRwLock.UnLock();
}

AutoWriteLock::AutoWriteLock(RWLock &rwLock): mRwLock(rwLock)
{
    mRwLock.WriteLock();
}

AutoWriteLock::~AutoWriteLock()
{
    mRwLock.UnLock();
}
