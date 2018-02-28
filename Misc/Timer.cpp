//////////////////////////////////////////////////////////////////////////
// Timer.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "Timer.h"
#include <stddef.h>
#include <unistd.h>
#include <sys/time.h>

Timer::Timer()
{
    mIsActive = false;
    mInterval = 0;
    mOnTimerCB = NULL;
    mOnTimerData = NULL;
    mTimerThread = NULL;
}

Timer::~Timer()
{
    mIsActive = false;
    mInterval = 0;
    mOnTimerCB = NULL;
    mOnTimerData = NULL;
    mTimerThread = NULL;
}

bool Timer::SetInterval(unsigned int seconds)
{
    if(seconds <= 0)
    {
        return false;
    }

    mInterval = seconds;
    return true;
}

unsigned int Timer::GetInterval() const
{
    return mInterval;
}

bool Timer::IsActive() const
{
    return mIsActive;
}

bool Timer::Start(void (*OnTimer)(void*), void* param)
{
    if(mInterval <= 0)
    {
        return false;
    }

    mOnTimerCB = OnTimer;
    if(mOnTimerCB == NULL)
    {
        return false;
    }

    mOnTimerData = param;

    // Start a new thread
    mTimerThread = NULL;
    int result = pthread_create(&mTimerThread, NULL, TimerThreadProc,
            (void *)this);
    if (result != 0)
    {
        return false;
    }

    return true;
}

void Timer::SetActive(bool isActive)
{
    mIsActive = isActive;
}

bool Timer::Stop()
{
    if(!IsActive())
    {
        return false;
    }

    // Stop thread
    SetActive(false);
    // Send a cancellation request to a thread
    int ret = pthread_cancel(mTimerThread);
    if(ret != 0)
    {
        return false;
    }
    // Join with a terminated thread
    void* threadRet = NULL;
    ret = pthread_join(mTimerThread, &threadRet);
    if(ret != 0)
    {
        return false;
    }

    mInterval = 0;
    mOnTimerCB = NULL;
    mOnTimerData = NULL;
    mTimerThread = NULL;

    return true;
}

void* Timer::TimerThreadProc(void* lpParam)
{
    int result = 0;
    // Set cancel state to be cancel enabled
    result = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (result != 0)
    {
        return (void*) NULL;
    }

    // Set the cancel type to deferred. Need call pthread_join to cancel it.
    result = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (result != 0)
    {
        return (void*) NULL;
    }

    // Do the actual work
    Timer* instance = static_cast<Timer*>(lpParam);
    if (instance == NULL)
    {
        return (void*)NULL;
    }

    instance->SetActive(true);
    while(instance->IsActive())
    {
        usleep(instance->GetInterval() * 1000 * 1000);
        if(instance->mOnTimerCB)
        {
            instance->mOnTimerCB(instance->mOnTimerData);
        }

#if 0
        timeval currentTime;
        gettimeofday(&currentTime, NULL);
        if((instance->GetInterval() * 1000) >= (unsigned int)((currentTime.tv_sec - instance->mStartTime.tv_sec) * 1000 + (currentTime.tv_usec - instance->mStartTime.tv_usec)/1000))
        {
            instance->mOnTimerCB(instance->mOnTimerData);
        }
        gettimeofday(&mStartTime, NULL);
#endif
    };
    instance->SetActive(false);

    pthread_exit(NULL);
}
