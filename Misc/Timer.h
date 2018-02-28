//////////////////////////////////////////////////////////////////////////
// Timer.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef Timer_INCLUDED
#define Timer_INCLUDED

#include <pthread.h>

class Timer
{
public:
    Timer();
    virtual ~Timer();

    // Set the interval of this timer
    bool SetInterval(unsigned int seconds);
    unsigned int GetInterval() const;
    // Check whether this timer is alive or not
    bool IsActive() const;
    // Start the timer, the first argument is the callback function,
    // The second argument is the data will be passed to the callback function
    bool Start(void (*OnTimer)(void*), void* param);
    // Stop the function
    bool Stop();

private:
    static void* TimerThreadProc(void* lpParam);
    void SetActive(bool isActive);

private:
    bool mIsActive;
    // Interval, in second
    unsigned int mInterval;
    // Callback function when timer hits
    void (*mOnTimerCB)(void*);
    // The data will be passed to the callback function
    void* mOnTimerData;
    pthread_t mTimerThread;
};

#endif // Timer_INCLUDED
