//////////////////////////////////////////////////////////////////////////
// Timespan.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef Timespan_INCLUDED
#define Timespan_INCLUDED

#include "Types.h"

class Timespan
{
public:
    // Creates a zero Timespan.
    Timespan();

    // Creates a Timespan.
    Timespan(Int64 microseconds);

    // Creates a Timespan. Useful for creating a Timespan from timeval.
    Timespan(int seconds, int microseconds);

    // Creates a Timespan.
    Timespan(int days, int hours, int minutes, int seconds, int microseconds);

    // Creates a Timespan from another one.
    Timespan(const Timespan& timespan);

    ~Timespan();

    Timespan& operator =(const Timespan& timespan);
    Timespan& operator =(Int64 microseconds);

    // Assigns a new span.
    Timespan& Assign(int days, int hours, int minutes, int seconds,
            int microseconds);

    // Assigns a new span. Useful for assigning from timeval.
    Timespan& Assign(int seconds, int microseconds);

    bool operator ==(const Timespan& ts) const
    {
        return mValue == ts.mValue;
    }
    bool operator !=(const Timespan& ts) const
    {
        return mValue != ts.mValue;
    }
    bool operator >(const Timespan& ts) const
    {
        return mValue > ts.mValue;
    }
    bool operator >=(const Timespan& ts) const
    {
        return mValue >= ts.mValue;
    }
    bool operator <(const Timespan& ts) const
    {
        return mValue < ts.mValue;
    }
    bool operator <=(const Timespan& ts) const
    {
        return mValue <= ts.mValue;
    }

    bool operator ==(Int64 microseconds) const
    {
        return mValue == microseconds;
    }
    bool operator !=(Int64 microseconds) const
    {
        return mValue != microseconds;
    }
    bool operator >(Int64 microseconds) const
    {
        return mValue > microseconds;
    }
    bool operator >=(Int64 microseconds) const
    {
        return mValue >= microseconds;
    }
    bool operator <(Int64 microseconds) const
    {
        return mValue < microseconds;
    }
    bool operator <=(Int64 microseconds) const
    {
        return mValue <= microseconds;
    }

    Timespan operator +(const Timespan& d) const;
    Timespan operator -(const Timespan& d) const;
    Timespan& operator +=(const Timespan& d);
    Timespan& operator -=(const Timespan& d);

    Timespan operator +(Int64 microseconds) const;
    Timespan operator -(Int64 microseconds) const;
    Timespan& operator +=(Int64 microseconds);
    Timespan& operator -=(Int64 microseconds);

    // Returns the number of days.
    int GetDays() const
    {
        return int(mValue / Day);
    }

    // Returns the number of hours (0 to 23).
    int GetHours() const
    {
        return int((mValue / Hour) % 24);
    }

    // Returns the total number of hours.
    int GetTotalHours() const
    {
        return int(mValue / Hour);
    }

    // Returns the number of minutes (0 to 59).
    int GetMinutes() const
    {
        return int((mValue / Minute) % 60);
    }

    // Returns the total number of minutes.
    int GetTotalMinutes() const
    {
        return int(mValue / Minute);
    }

    /// Returns the number of seconds (0 to 59).
    int GetSeconds() const
    {
        return int((mValue / Second) % 60);
    }

    // Returns the total number of seconds.
    int GetTotalSeconds() const
    {
        return int(mValue / Second);
    }

    // Returns the number of milliseconds (0 to 999).
    int GetMilliseconds() const
    {
        return int((mValue / Millisecond) % 1000);
    }

    // Returns the total number of milliseconds.
    Int64 GetTotalMilliseconds() const
    {
        return mValue / Millisecond;
    }

    // Returns the fractions of a millisecond in microseconds (0 to 999).
    int GetMicroseconds() const
    {
        return int(mValue % 1000);
    }

    // Returns the fractions of a second in microseconds (0 to 999999).
    int GetUseconds() const
    {
        return int(mValue % 1000000);
    }

    // Returns the total number of microseconds.
    Int64 GetTotalMicroseconds() const
    {
        return mValue;
    }

    // The number of microseconds in a millisecond.
    static const Int64 Millisecond;
    // The number of microseconds in a second.
    static const Int64 Second;
    // The number of microseconds in a minute.
    static const Int64 Minute;
    // The number of microseconds in a hour.
    static const Int64 Hour;
    // The number of microseconds in a day.
    static const Int64 Day;

private:
    Int64 mValue;
};

#endif // Timespan_INCLUDED
