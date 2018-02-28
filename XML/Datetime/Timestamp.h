//////////////////////////////////////////////////////////////////////////
// Timestamp.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef Timestamp_INCLUDED
#define Timestamp_INCLUDED

#include <ctime>
#include "Types.h"

class Timestamp
{
public:
    // Creates a timestamp with the current time.
    Timestamp();
    // Creates a timestamp from the given time value.
    Timestamp(Int64 value);
    Timestamp(const Timestamp& other);
    ~Timestamp();

    Timestamp& operator =(const Timestamp& other);
    Timestamp& operator =(Int64 value);

    // Updates the Timestamp with the current time.
    void Update();

    bool operator ==(const Timestamp& ts) const
    {
        return mValue == ts.mValue;
    }
    bool operator !=(const Timestamp& ts) const
    {
        return mValue != ts.mValue;
    }
    bool operator >(const Timestamp& ts) const
    {
        return mValue > ts.mValue;
    }
    bool operator >=(const Timestamp& ts) const
    {
        return mValue >= ts.mValue;
    }
    bool operator <(const Timestamp& ts) const
    {
        return mValue < ts.mValue;
    }
    bool operator <=(const Timestamp& ts) const
    {
        return mValue <= ts.mValue;
    }
    Timestamp operator +(Int64 microseconds) const
    {
        return Timestamp(mValue + microseconds);
    }
    Timestamp operator -(Int64 microseconds) const
    {
        return Timestamp(mValue - microseconds);
    }
    Int64 operator -(const Timestamp& ts) const
    {
        return mValue - ts.mValue;
    }
    Timestamp& operator +=(Int64 microseconds)
    {
        mValue += microseconds;
        return *this;
    }
    Timestamp& operator -=(Int64 microseconds)
    {
        mValue -= microseconds;
        return *this;
    }

    // Returns the timestamp expressed in time_t.
    // time_t base time is midnight, January 1, 1970.
    // Resolution is one second.
    std::time_t GetEpochTime() const
    {
        return std::time_t(mValue / GetResolution());
    }

    // Returns the timestamp expressed in UTC-based time. 
    // UTC base time is midnight, October 15, 1582.
    // Resolution is 100 nanoseconds.
    Int64 GetUTCTime() const
    {
        return mValue * 10 + (Int64(0x01b21dd2) << 32) + 0x13814000;
    }

    // Returns the timestamp expressed in microseconds
    // since the Unix epoch, midnight, January 1, 1970.
    Int64 GetEpochMicroseconds() const
    {
        return mValue;
    }

    // Returns the microseconds elapsed since the time denoted by the timestamp.
    Int64 GetElapsed() const
    {
        Timestamp now;
        return now - *this;
    }

    // Returns true if the given interval has passed since the time denoted by the timestamp.
    bool IsElapsed(Int64 microseconds) const
    {
        Timestamp now;
        Int64 diff = now - *this;
        return diff >= microseconds;
    }

    // Creates a timestamp from a std::time_t.
    static Timestamp FromEpochTime(std::time_t value);

    // Creates a timestamp from a UTC time value.
    // in 100 nanosecond resolution
    static Timestamp FromUTCTime(Int64 value);

    // Returns the resolution in units per second.
    // Since the timestamp has microsecond resolution, the returned value is always 1000000.
    static Int64 GetResolution()
    {
        return 1000000;
    }

private:
    // Monotonic UTC time value in microsecond resolution
    Int64 mValue;
};

#endif // Timestamp_INCLUDED
