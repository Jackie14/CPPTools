/*
 * RWLock.h
 *
 */

#ifndef RWLOCK_H_
#define RWLOCK_H_

#include <pthread.h>

class RWLock
{
public:
    RWLock();
    ~RWLock();

    void ReadLock();
    void WriteLock();
    void UnLock();

private:
    RWLock(const RWLock &rwLock);
    const RWLock & operator=(const RWLock &rwLock);

private:
    pthread_rwlock_t mRwLock;
};

class AutoReadLock
{
public:
    AutoReadLock(RWLock &rwlock);
    ~AutoReadLock();

private:
    RWLock &mRwLock;
};

class AutoWriteLock
{
public:
    AutoWriteLock(RWLock &rwlock);
    ~AutoWriteLock();

private:
    RWLock &mRwLock;
};

#endif /* RWLOCK_H_ */
