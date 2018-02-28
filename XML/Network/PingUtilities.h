//////////////////////////////////////////////////////////////////////////
// PingUtilities.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef PingUtilities_INCLUDED
#define PingUtilities_INCLUDED

#include <string>

class PingUtilities
{
public:
    static bool Ping(const std::string& target, int maxCount = 4);
    static unsigned short CalChecksum(unsigned short* addr, unsigned int len);
};

#endif // PingUtilities_INCLUDED
