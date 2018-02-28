//////////////////////////////////////////////////////////////////////////
// DateTime.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef DateTime_INCLUDED
#define DateTime_INCLUDED

#include "Types.h"
#include "Timestamp.h"
#include "Timespan.h"

// Symbolic names for month numbers (1 to 12).
enum Months
{
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};

// Symbolic names for week day numbers (0 to 6).
enum DaysOfWeek
{
    Sunday = 0,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};

//////////////////////////////////////////////////////////////////////////
// This class represents an instant in time, expressed in years, months, days, hours,
// minutes, seconds and milliseconds based on the Gregorian calendar.
class DateTime
{
public:
    // Creates a DateTime for the current date and time.
    DateTime();

    // Creates a DateTime for the date and time given in a Timestamp.
    DateTime(const Timestamp& ts);

    // Creates a DateTime for the given Gregorian date and time.
    //   * year is from 0 to 9999.
    //   * month is from 1 to 12.
    //   * day is from 1 to 31.
    //   * hour is from 0 to 23.
    //   * minute is from 0 to 59.
    //   * second is from 0 to 59.
    //   * millisecond is from 0 to 999.
    //   * microsecond is from 0 to 999.
    DateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, 
        int millisecond = 0, int microsecond = 0);

    // Creates a DateTime for the given Julian day.
    DateTime(double julianDay);

    // Creates a DateTime from an UtcTimeVal and a TimeDiff.
    DateTime(Int64 utcTime, Int64 microseconds);

    DateTime(const DateTime& dateTime);

    ~DateTime();

    DateTime& operator = (const DateTime& dateTime);

    // Assigns a Timestamp.
    DateTime& operator = (const Timestamp& ts);

    // Assigns a Julian day.
    DateTime& operator = (double julianDay);

    // Assigns a Gregorian date and time.
    DateTime& Assign(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, 
        int millisecond = 0, int microseconds = 0);

    // Returns the year.
    int Year() const
    {
        return mYear;
    }

    // Returns the month (1 to 12).
    int Month() const
    {
        return mMonth;
    }

    // Returns the week number within the year.
    // FirstDayOfWeek should be either Sunday (0) or Monday (1).
    int Week(int firstDayOfWeek = Monday) const;

    // Returns the day within the month (1 to 31).
    int Day() const
    {
        return mDay;
    }

    // Returns the weekday (0 to 6, where 0 = Sunday, 1 = Monday, ..., 6 = Saturday).
    int DayOfWeek() const;

    // Returns the number of the day in the year.
    // January 1 is 1, February 1 is 32, etc.
    int DayOfYear() const;

    // Returns the hour (0 to 23).
    int Hour() const
    {
        return mHour;
    }

    // Returns the hour (0 to 12).
    int HourAMPM() const
    {
        if (mHour < 1)
        {
            return 12;
        }
        else if (mHour > 12)
        {
            return mHour - 12;
        }
        else
        {
            return mHour;
        }
    }

    // Returns true if hour < 12;
    bool IsAM() const
    {
        return mHour < 12;
    }

    // Returns true if hour >= 12.
    bool IsPM() const
    {
        return mHour >= 12;
    }

    // Returns the minute (0 to 59).
    int Minute() const
    {
        return mMinute;
    }

    // Returns the second (0 to 59).
    int Second() const
    {
        return mSecond;
    }

    // Returns the millisecond (0 to 999)
    int Millisecond() const
    {
        return mMillisecond;
    }

    // Returns the microsecond (0 to 999)
    int Microsecond() const
    {
        return mMicrosecond;
    }

    // Returns the julian day for the date and time.
    double JulianDay() const;

    // Returns the date and time expressed as a Timestamp.
    Timestamp GetTimestamp() const
    {
        return Timestamp::FromUTCTime(mUTCTime);
    }

    // Returns the date and time expressed in UTC-based
    // time. UTC base time is midnight, October 15, 1582.
    // Resolution is 100 nanoseconds.
    Int64 UTCTime() const
    {
        return mUTCTime;
    }

    bool operator == (const DateTime& dateTime) const
    {
        return mUTCTime == dateTime.mUTCTime;
    }
    bool operator != (const DateTime& dateTime) const
    {
        return mUTCTime != dateTime.mUTCTime;
    }
    bool operator <  (const DateTime& dateTime) const
    {
        return mUTCTime < dateTime.mUTCTime;
    }
    bool operator <= (const DateTime& dateTime) const
    {
        return mUTCTime <= dateTime.mUTCTime;
    }
    bool operator >  (const DateTime& dateTime) const
    {
        return mUTCTime > dateTime.mUTCTime;
    }
    bool operator >= (const DateTime& dateTime) const
    {
        return mUTCTime >= dateTime.mUTCTime;
    }

    DateTime  operator +  (const Timespan& span) const;
    DateTime  operator -  (const Timespan& span) const;
    Timespan  operator -  (const DateTime& dateTime) const;
    DateTime& operator += (const Timespan& span);
    DateTime& operator -= (const Timespan& span);

    // Converts a local time into UTC
    void MakeUTC(int tzd);

    // Converts a UTC time into a local time
    void MakeLocal(int tzd);

    // Returns true if the given year is a leap year
    static bool IsLeapYear(int year)
    {
        return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
    }

    // Returns the number of days in the given month and year.
    // Month is from 1 to 12.
    static int DaysOfMonth(int year, int month);

    // Checks if the given date and time is valid
    static bool IsValid(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, 
        int millisecond = 0, int microsecond = 0);

protected:	
    // Computes the Julian day for an UTC time.
    static double ToJulianDay(Int64 utcTime);

    // Computes the Julian day for a Gregorian calendar date and time.
    static double ToJulianDay(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, 
        int millisecond = 0, int microsecond = 0);

    // Computes the UTC time for a Julian day.
    static Int64 ToUTCTime(double julianDay);

    // Computes the Gregorian date for the given Julian day.
    void ComputeGregorian(double julianDay);

    // Extracts the daytime (hours, minutes, seconds, etc.) from the stored utcTime.
    void ComputeDaytime();

private:
    void CheckLimit(short& lower, short& higher, short limit);
    // Utility functions used to correct the overflow in compute Gregorian
    void Normalize();

    Int64 mUTCTime;
    short  mYear;
    short  mMonth;
    short  mDay;
    short  mHour;
    short  mMinute;
    short  mSecond;
    short  mMillisecond;
    short  mMicrosecond;
};

#endif // DateTime_INCLUDED
