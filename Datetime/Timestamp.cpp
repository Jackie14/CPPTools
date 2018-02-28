//////////////////////////////////////////////////////////////////////////
// Timestamp.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "Timestamp.h"
#include <algorithm>
#include "Log.h"
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>

//////////////////////////////////////////////////////////////////////////
Timestamp::Timestamp()
{
    Update();
}

Timestamp::Timestamp(Int64 value)
{
    mValue = value;
}

Timestamp::Timestamp(const Timestamp& other)
{
    mValue = other.mValue;
}

Timestamp::~Timestamp()
{
}

Timestamp& Timestamp::operator =(const Timestamp& other)
{
    mValue = other.mValue;
    return *this;
}

Timestamp& Timestamp::operator =(Int64 value)
{
    mValue = value;
    return *this;
}

Timestamp Timestamp::FromEpochTime(std::time_t value)
{
    return Timestamp(Int64(value) * GetResolution());
}

// Monotonic UTC time value in 100 nanosecond resolution
Timestamp Timestamp::FromUTCTime(Int64 value)
{
    value -= (Int64(0x01b21dd2) << 32) + 0x13814000;
    value /= 10;
    return Timestamp(value);
}

//////////////////////////////////////////////////////////////////////////
// Platform specific code
void Timestamp::Update()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL))
    {
        LOG(LogError, "Cannot get time of day");
        return;
    }
    mValue = Int64(tv.tv_sec) * GetResolution() + tv.tv_usec;
}

