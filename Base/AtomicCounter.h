//////////////////////////////////////////////////////////////////////////
// AtomicCounter.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef AtomicCounter_INCLUDED
#define AtomicCounter_INCLUDED

class AtomicCounter
{
public:
    AtomicCounter(int initialValue = 0)
    {
        mCounter = initialValue;
    }

    ~AtomicCounter()
    {
    }

    int Value() const
    {
        return mCounter;
    }

    int Increase()
    {
        int __i = 1;
        asm ("lock; xaddl %0, %1" : "+r" (__i), "+m" (mCounter) : : "memory");
        return ++__i;
    }

    int Decrease()
    {
        int __i = -1;
        asm ("lock; xaddl %0, %1" : "+r" (__i), "+m" (mCounter) : : "memory");
        return --__i;
    }

    // prefix
    int operator ++()
    {
        return Increase();
    }

    // postfix
    int operator ++(int)
    {
        return Increase();
    }

    // prefix
    int operator --()
    {
        return Decrease();
    }

    // postfix
    int operator --(int)
    {
        return Decrease();
    }

private:
    int mCounter;
};

#endif // AtomicCounter_INCLUDED
