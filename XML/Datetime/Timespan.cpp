//////////////////////////////////////////////////////////////////////////
// Timespan.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "Timespan.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
const Int64 Timespan::Millisecond = 1000;
const Int64 Timespan::Second = 1000 * Timespan::Millisecond;
const Int64 Timespan::Minute = 60 * Timespan::Second;
const Int64 Timespan::Hour = 60 * Timespan::Minute;
const Int64 Timespan::Day = 24 * Timespan::Hour;

//////////////////////////////////////////////////////////////////////////
Timespan::Timespan() :
    mValue(0)
{
}

Timespan::Timespan(Int64 microseconds) :
    mValue(microseconds)
{
}

Timespan::Timespan(int seconds, int microseconds) :
    mValue(Int64(seconds) * Second + microseconds)
{
}

Timespan::Timespan(int days, int hours, int minutes, int seconds,
        int microseconds) :
    mValue(Int64(microseconds) + Int64(seconds) * Second + Int64(minutes)
            * Minute + Int64(hours) * Hour + Int64(days) * Day)
{
}

Timespan::Timespan(const Timespan& timespan) :
    mValue(timespan.mValue)
{
}

Timespan::~Timespan()
{
}

Timespan& Timespan::operator =(const Timespan& timespan)
{
    mValue = timespan.mValue;
    return *this;
}

Timespan& Timespan::operator =(Int64 microseconds)
{
    mValue = microseconds;
    return *this;
}

Timespan& Timespan::Assign(int days, int hours, int minutes, int seconds,
        int microseconds)
{
    mValue = Int64(microseconds) + Int64(seconds) * Second + Int64(minutes)
            * Minute + Int64(hours) * Hour + Int64(days) * Day;
    return *this;
}

Timespan& Timespan::Assign(int seconds, int microseconds)
{
    mValue = Int64(seconds) * Second + Int64(microseconds);
    return *this;
}

Timespan Timespan::operator +(const Timespan& d) const
{
    return Timespan(mValue + d.mValue);
}

Timespan Timespan::operator -(const Timespan& d) const
{
    return Timespan(mValue - d.mValue);
}

Timespan& Timespan::operator +=(const Timespan& d)
{
    mValue += d.mValue;
    return *this;
}

Timespan& Timespan::operator -=(const Timespan& d)
{
    mValue -= d.mValue;
    return *this;
}

Timespan Timespan::operator +(Int64 microseconds) const
{
    return Timespan(mValue + microseconds);
}

Timespan Timespan::operator -(Int64 microseconds) const
{
    return Timespan(mValue - microseconds);
}

Timespan& Timespan::operator +=(Int64 microseconds)
{
    mValue += microseconds;
    return *this;
}

Timespan& Timespan::operator -=(Int64 microseconds)
{
    mValue -= microseconds;
    return *this;
}
