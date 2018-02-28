/*
 * StateHistory.h
 *
 *  Created on: Mar 11, 2014
 *      Author: myan
 */

#ifndef STATEHISTORY_H_
#define STATEHISTORY_H_
#include <deque>
#include "ServerState.h"
#include "CriticalSection.h"
/**
 * This class use STL dequeue to store the history state of Database and agent.
 */
template<class T>
class StateHistory
{
public:
    StateHistory(size_t maxStates);
    virtual ~StateHistory();

    /** Whether the history record is empty.*/
    bool IsEmpty() const;

    /** Current number of state record.*/
    size_t GetStates() const;

    /** Get the maximum number of state records. */
    size_t GetMaxStates() const
    {
        return mMaxStates;
    }

    /** Add the state to the history.*/
    void AddState(const T& state);

    /**Get the earliest state. */
    bool GetEarliestState(T& outState) const;

    /** Get the latest state. */
    bool GetLatestState(T& outState) const;

    /**Get the earliest state and removed from the dequeue. */
    bool PopEarliestState(T& outState);

    /** Get the latest state and removed form the dequeue. */
    bool PopLatestState(T& outState);

    /** Get a copy of the state queue.*/
    const std::deque<T> GetStateDeque() const;

    /** Reset dequeue.*/
    void Reset();

private:
    size_t mMaxStates;
    std::deque<T> mStateQueue;

    mutable CriticalSection mCriticalSection;
};

template<class T> StateHistory<T>::StateHistory(size_t maxStates)
{
    mMaxStates = (maxStates > 0) ? maxStates : 1000;
}

template<class T> StateHistory<T>::~StateHistory()
{
    mStateQueue.clear();
}

template<class T>
size_t StateHistory<T>::GetStates() const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    return mStateQueue.size();
}

template<class T>
void StateHistory<T>::AddState(const T& state)
{
    AutoCriticalSection autoLock(&mCriticalSection);
    while (mStateQueue.size() >= mMaxStates)
    {
        mStateQueue.pop_front();
    }
    mStateQueue.push_back(state);
}

template<class T>
bool StateHistory<T>::GetEarliestState(T& outState) const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    if (mStateQueue.empty())
    {
        return false;
    }
    outState = mStateQueue.front();
    return true;
}

template<class T>
bool StateHistory<T>::GetLatestState(T& outState) const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    if (mStateQueue.empty())
    {
        return false;
    }
    outState = mStateQueue.back();
    return true;
}

template<class T>
bool StateHistory<T>::PopEarliestState(T& outState)
{
    AutoCriticalSection autoLock(&mCriticalSection);
    if (mStateQueue.empty())
    {
        return false;
    }
    outState = mStateQueue.front();
    mStateQueue.pop_front();
    return true;
}

template<class T>
bool StateHistory<T>::PopLatestState(T& outState)
{
    AutoCriticalSection autoLock(&mCriticalSection);
    if (mStateQueue.empty())
    {
        return false;
    }
    outState = mStateQueue.back();
    mStateQueue.pop_back();
    return true;
}

template<class T>
const std::deque<T> StateHistory<T>::GetStateDeque() const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    return mStateQueue;
}

template<class T>
void StateHistory<T>::Reset()
{
    AutoCriticalSection autoLock(&mCriticalSection);
    mStateQueue.clear();
}

template<class T>
bool StateHistory<T>::IsEmpty() const
{
    AutoCriticalSection autoLock(&mCriticalSection);
    return mStateQueue.empty();
}
#endif /* STATEHISTORY_H_ */
