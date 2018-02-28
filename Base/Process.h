//////////////////////////////////////////////////////////////////////////
// Process.h
// 
//////////////////////////////////////////////////////////////////////////

#ifndef Process_INCLUDED
#define Process_INCLUDED

#include <unistd.h>

namespace Process
{
    static inline int AtomInc(int *var)
    {
        int __i = 1;
        asm ("lock; xaddl %0, %1" : "+r" (__i), "+m" (*var) : : "memory");
        return ++__i;
    }

    static inline int AtomDec(int *var)
    {
        int __i = -1;
        asm ("lock; xaddl %0, %1" : "+r" (__i), "+m" (*var) : : "memory");
        return --__i;
    }

    void Sleep( unsigned long ms );
}

#endif // Process_INCLUDED
