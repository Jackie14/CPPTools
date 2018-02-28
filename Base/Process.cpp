//////////////////////////////////////////////////////////////////////////
// Process.cpp
//
//////////////////////////////////////////////////////////////////////////

#include "Process.h"

namespace Process
{
    void Sleep( unsigned long ms )
    {
        unsigned long seconds = ms / 1000;
        if (seconds == 0)
        {
            seconds = 1;
        }
        
        ::sleep( seconds );
    }
}

