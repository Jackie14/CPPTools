//////////////////////////////////////////////////////////////////////////
// DateTime.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "DateTime.h"
#include <algorithm>
#include <cmath>

inline double DateTime::ToJulianDay(Int64 utcTime)
{
    double utcDays = double(utcTime)/864000000000.0;
    // first day of Gregorian reform (Oct 15 1582)
    return utcDays + 2299160.5;
}

inline Int64 DateTime::ToUTCTime(double julianDay)
{
    return Int64((julianDay - 2299160.5)*864000000000.0);
}

DateTime::DateTime()
{
    Timestamp now;
    mUTCTime = now.GetUTCTime();
    ComputeGregorian(JulianDay());
    ComputeDaytime();
}

DateTime::DateTime(const Timestamp& ts):
mUTCTime(ts.GetUTCTime())
{
    ComputeGregorian(JulianDay());
    ComputeDaytime();
}

DateTime::DateTime(int year, int month, int day, int hour, int minute,
        int second, int millisecond, int microsecond):
mYear(year),
mMonth(month),
mDay(day),
mHour(hour),
mMinute(minute),
mSecond(second),
mMillisecond(millisecond),
mMicrosecond(microsecond)
{
    mUTCTime = ToUTCTime(ToJulianDay(year, month, day)) + 10*(hour*Timespan::Hour + 
        minute*Timespan::Minute + second*Timespan::Second + 
        millisecond*Timespan::Millisecond + microsecond);
}

DateTime::DateTime(double julianDay):
mUTCTime(ToUTCTime(julianDay))
{
    ComputeGregorian(julianDay);
}

DateTime::DateTime(Int64 utcTime, Int64 microseconds):
mUTCTime(utcTime + microseconds*10)
{
    ComputeGregorian(JulianDay());
    ComputeDaytime();
}

DateTime::DateTime(const DateTime& dateTime):
mUTCTime(dateTime.mUTCTime),
mYear(dateTime.mYear),
mMonth(dateTime.mMonth),
mDay(dateTime.mDay),
mHour(dateTime.mHour),
mMinute(dateTime.mMinute),
mSecond(dateTime.mSecond),
mMillisecond(dateTime.mMillisecond),
mMicrosecond(dateTime.mMicrosecond)
{
}

DateTime::~DateTime()
{
}

DateTime& DateTime::operator = (const DateTime& dateTime)
{
    if (&dateTime != this)
    {
        mUTCTime     = dateTime.mUTCTime;
        mYear        = dateTime.mYear;
        mMonth       = dateTime.mMonth;
        mDay         = dateTime.mDay;
        mHour        = dateTime.mHour;
        mMinute      = dateTime.mMinute;
        mSecond      = dateTime.mSecond;
        mMillisecond = dateTime.mMillisecond;
        mMicrosecond = dateTime.mMicrosecond;
    }
    return *this;
}

DateTime& DateTime::operator = (const Timestamp& ts)
{
    mUTCTime = ts.GetUTCTime();
    ComputeGregorian(JulianDay());
    ComputeDaytime();
    return *this;
}

DateTime& DateTime::operator = (double julianDay)
{
    mUTCTime = ToUTCTime(julianDay);
    ComputeGregorian(julianDay);
    return *this;
}

DateTime& DateTime::Assign(int year, int month, int day, int hour,
        int minute, int second, int millisecond, int microsecond)
{
    mUTCTime = ToUTCTime(ToJulianDay(year, month, day)) + 10*(hour*Timespan::Hour + 
        minute*Timespan::Minute + second*Timespan::Second + 
        millisecond*Timespan::Millisecond + microsecond);
    mYear = year;
    mMonth = month;
    mDay = day;
    mHour = hour;
    mMinute = minute;
    mSecond = second;
    mMillisecond = millisecond;
    mMicrosecond = microsecond;

    return *this;
}

int DateTime::DayOfWeek() const
{
    return int((std::floor(JulianDay() + 1.5))) % 7;
}

int DateTime::DayOfYear() const
{
    int doy = 0;
    for (int month = 1; month < mMonth; ++month)
    {
        doy += DaysOfMonth(mYear, month);
    }
    doy += mDay;
    return doy;
}

int DateTime::DaysOfMonth(int year, int month)
{
    static int daysOfMonthTable[] = {0, 31, 28, 31, 30, 31, 30, 31,
            31, 30, 31, 30, 31};

    if (month == 2 && IsLeapYear(year))
    {
        return 29;
    }
    else
    {
        return daysOfMonthTable[month];
    }
}

bool DateTime::IsValid(int year, int month, int day, int hour,
        int minute, int second, int millisecond, int microsecond)
{
    return
        (year >= 0 && year <= 9999) &&
        (month >= 1 && month <= 12) &&
        (day >= 1 && day <= DaysOfMonth(year, month)) &&
        (hour >= 0 && hour <= 23) &&
        (minute >= 0 && minute <= 59) &&
        (second >= 0 && second <= 59) &&
        (millisecond >= 0 && millisecond <= 999) &&
        (microsecond >= 0 && microsecond <= 999);
}

int DateTime::Week(int firstDayOfWeek) const
{
    // find the first firstDayOfWeek.
    int baseDay = 1;
    while (DateTime(mYear, 1, baseDay).DayOfWeek() != firstDayOfWeek)
    {
        ++baseDay;
    }

    int doy  = DayOfYear();
    int offs = baseDay <= 4 ? 0 : 1; 
    if (doy < baseDay)
    {
        return offs;
    }
    else
    {
        return (doy - baseDay)/7 + 1 + offs;
    }
}

double DateTime::JulianDay() const
{
    return ToJulianDay(mUTCTime);
}

DateTime DateTime::operator + (const Timespan& span) const
{
    return DateTime(mUTCTime, span.GetTotalMicroseconds());
}

DateTime DateTime::operator - (const Timespan& span) const
{
    return DateTime(mUTCTime, -span.GetTotalMicroseconds());
}

Timespan DateTime::operator - (const DateTime& dateTime) const
{
    return Timespan((mUTCTime - dateTime.mUTCTime)/10);
}

DateTime& DateTime::operator += (const Timespan& span)
{
    mUTCTime += span.GetTotalMicroseconds()*10;
    ComputeGregorian(JulianDay());
    ComputeDaytime();
    return *this;
}

DateTime& DateTime::operator -= (const Timespan& span)
{
    mUTCTime -= span.GetTotalMicroseconds()*10;
    ComputeGregorian(JulianDay());
    ComputeDaytime();
    return *this;
}

void DateTime::MakeUTC(int tzd)
{
    operator -= (Timespan(((Int64) tzd)*Timespan::Second));
}

void DateTime::MakeLocal(int tzd)
{
    operator += (Timespan(((Int64) tzd)*Timespan::Second));
}

double DateTime::ToJulianDay(int year, int month, int day, int hour,
        int minute, int second, int millisecond, int microsecond)
{
    // lookup table for (153*month - 457)/5 - note that 3 <= month <= 14.
    static int lookup[] = {-91, -60, -30, 0, 31, 61, 92, 122, 153,
            184, 214, 245, 275, 306, 337};

    // day to double
    double dday = double(day) + ((double((hour*60 + minute)*60 + second)*1000 +
            millisecond)*1000 + microsecond)/86400000000.0;
    if (month < 3)
    {
        month += 12;
        --year;
    }
    double dyear = double(year);
    return dday + lookup[month] + 365*year + std::floor(dyear/4) -
            std::floor(dyear/100) + std::floor(dyear/400) + 1721118.5;
}

void DateTime::CheckLimit(short& lower, short& higher, short limit)
{
    if (lower >= limit)
    {
        higher += short(lower / limit);
        lower   = short(lower % limit);
    }
}

void DateTime::Normalize()
{
    CheckLimit(mMicrosecond, mMillisecond, 1000);
    CheckLimit(mMillisecond, mSecond, 1000);
    CheckLimit(mSecond, mMinute, 60);
    CheckLimit(mMinute, mHour, 60);
    CheckLimit(mHour, mDay, 24);

    if (mDay > DaysOfMonth(mYear, mMonth))
    {
        mDay -= DaysOfMonth(mYear, mMonth);
        if (++mMonth > 12)
        {
            ++mYear;
            mMonth -= 12;
        }
    }
}

void DateTime::ComputeGregorian(double julianDay)
{
    double z    = std::floor(julianDay - 1721118.5);
    double r    = julianDay - 1721118.5 - z;
    double g    = z - 0.25;
    double a    = std::floor(g / 36524.25);
    double b    = a - std::floor(a/4);
    mYear       = short(std::floor((b + g)/365.25));
    double c    = b + z - std::floor(365.25*mYear);
    mMonth      = short(std::floor((5*c + 456)/153));
    double dday = c - std::floor((153.0*mMonth - 457)/5) + r;
    mDay        = short(dday);
    if (mMonth > 12)
    {
        ++mYear;
        mMonth -= 12;
    }
    r      *= 24;
    mHour   = short(std::floor(r));
    r      -= std::floor(r);
    r      *= 60;
    mMinute = short(std::floor(r));
    r      -= std::floor(r);
    r      *= 60;
    mSecond = short(std::floor(r));
    r      -= std::floor(r);
    r      *= 1000;
    mMillisecond = short(std::floor(r));
    r      -= std::floor(r);
    r      *= 1000;
    mMicrosecond = short(r + 0.5);

    Normalize();
}

void DateTime::ComputeDaytime()
{
    Timespan span(mUTCTime/10);
    mHour        = span.GetHours();
    mMinute      = span.GetMinutes();
    mSecond      = span.GetSeconds();
    mMillisecond = span.GetMilliseconds();
    mMicrosecond = span.GetMicroseconds();
}
