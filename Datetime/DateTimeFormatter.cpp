//////////////////////////////////////////////////////////////////////////
// DateTimeFormatter.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "DateTimeFormatter.h"
#include "Timestamp.h"
#include "NumberFormatter.h"

//////////////////////////////////////////////////////////////////////////
const std::string DateTimeFormat::ISO8601_FORMAT  = "%Y-%m-%dT%H:%M:%S%z";
const std::string DateTimeFormat::RFC822_FORMAT   = "%w, %e %b %y %H:%M:%S %Z";
const std::string DateTimeFormat::RFC1123_FORMAT  = "%w, %e %b %Y %H:%M:%S %Z";
const std::string DateTimeFormat::HTTP_FORMAT     = "%w, %d %b %Y %H:%M:%S %Z";
const std::string DateTimeFormat::RFC850_FORMAT   = "%W, %e-%b-%y %H:%M:%S %Z";
const std::string DateTimeFormat::RFC1036_FORMAT  = "%W, %e %b %y %H:%M:%S %Z";
const std::string DateTimeFormat::ASCTIME_FORMAT  = "%w %b %f %H:%M:%S %Y";
const std::string DateTimeFormat::SORTABLE_FORMAT = "%Y-%m-%d %H:%M:%S";

const std::string DateTimeFormat::WEEKDAY_NAMES[] =
{
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

const std::string DateTimeFormat::MONTH_NAMES[] =
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

//////////////////////////////////////////////////////////////////////////
void DateTimeFormatter::Append(std::string& str, const DateTime& dateTime, 
                               const std::string& fmt, int timeZoneDifferential)
{
    std::string::const_iterator it  = fmt.begin();
    std::string::const_iterator end = fmt.end();
    while (it != end)
    {
        if (*it == '%')
        {
            if (++it != end)
            {
                switch (*it)
                {
                case 'w': 
                    str.append(DateTimeFormat::WEEKDAY_NAMES[dateTime.DayOfWeek()], 0, 3);
                    break;

                case 'W': 
                    str.append(DateTimeFormat::WEEKDAY_NAMES[dateTime.DayOfWeek()]); 
                    break;

                case 'b': 
                    str.append(DateTimeFormat::MONTH_NAMES[dateTime.Month() - 1], 0, 3);
                    break;

                case 'B': 
                    str.append(DateTimeFormat::MONTH_NAMES[dateTime.Month() - 1]); 
                    break;

                case 'd':
                    NumberFormatter::Append0(str, dateTime.Day(), 2); 
                    break;

                case 'e': 
                    NumberFormatter::Append(str, dateTime.Day()); 
                    break;

                case 'f': 
                    NumberFormatter::Append(str, dateTime.Day(), 2); 
                    break;

                case 'm': 
                    NumberFormatter::Append0(str, dateTime.Month(), 2); 
                    break;

                case 'n': 
                    NumberFormatter::Append(str, dateTime.Month());
                    break;

                case 'o':
                    NumberFormatter::Append(str, dateTime.Month(), 2);
                    break;

                case 'y': 
                    NumberFormatter::Append0(str, dateTime.Year() % 100, 2);
                    break;

                case 'Y': 
                    NumberFormatter::Append0(str, dateTime.Year(), 4);
                    break;

                case 'H': 
                    NumberFormatter::Append0(str, dateTime.Hour(), 2);
                    break;

                case 'h': 
                    NumberFormatter::Append0(str, dateTime.HourAMPM(), 2);
                    break;

                case 'a':
                    str.append(dateTime.IsAM() ? "am" : "pm");
                    break;

                case 'A': 
                    str.append(dateTime.IsAM() ? "AM" : "PM");
                    break;

                case 'M': 
                    NumberFormatter::Append0(str, dateTime.Minute(), 2); 
                    break;

                case 'S':
                    NumberFormatter::Append0(str, dateTime.Second(), 2);
                    break;

                case 'i': 
                    NumberFormatter::Append0(str, dateTime.Millisecond(), 3);
                    break;

                case 'c': 
                    NumberFormatter::Append(str, dateTime.Millisecond()/100); 
                    break;

                case 'F': 
                    NumberFormatter::Append0(str, dateTime.Millisecond()*1000 + dateTime.Microsecond(), 6);
                    break;

                case 'z': 
                    TZDISO(str, timeZoneDifferential);
                    break;

                case 'Z': 
                    TZDRFC(str, timeZoneDifferential);
                    break;

                default: 
                    str += *it;
                    break;
                }
                ++it;
            }
        }
        else str += *it++;
    }
}

void DateTimeFormatter::Append(std::string& str, const Timespan& timespan,
        const std::string& fmt)
{
    std::string::const_iterator it  = fmt.begin();
    std::string::const_iterator end = fmt.end();
    while (it != end)
    {
        if (*it == '%')
        {
            if (++it != end)
            {
                switch (*it)
                {
                case 'd': 
                    NumberFormatter::Append(str, timespan.GetDays());
                    break;

                case 'H':
                    NumberFormatter::Append0(str, timespan.GetHours(), 2);
                    break;

                case 'h': 
                    NumberFormatter::Append(str, timespan.GetTotalHours());
                    break;

                case 'M':
                    NumberFormatter::Append0(str, timespan.GetMinutes(), 2);
                    break;

                case 'm': 
                    NumberFormatter::Append(str, timespan.GetTotalMinutes());
                    break;

                case 'S':
                    NumberFormatter::Append0(str, timespan.GetSeconds(), 2);
                    break;

                case 's': 
                    NumberFormatter::Append(str, timespan.GetTotalSeconds());
                    break;

                case 'i': 
                    NumberFormatter::Append0(str, timespan.GetMilliseconds(), 3);
                    break;

                case 'c': 
                    NumberFormatter::Append(str, timespan.GetMilliseconds()/100);
                    break;

                case 'F': 
                    NumberFormatter::Append0(str, timespan.GetMilliseconds()*1000 +
                            timespan.GetMicroseconds(), 6);
                    break;

                default: 
                    str += *it;
                    break;
                }
                ++it;
            }
        }
        else 
        {
            str += *it++;
        }
    }
}

void DateTimeFormatter::TZDISO(std::string& str, int timeZoneDifferential)
{
    if (timeZoneDifferential != UTC)
    {
        if (timeZoneDifferential >= 0)
        {
            str += '+';
            NumberFormatter::Append0(str, timeZoneDifferential/3600, 2);
            str += ':';
            NumberFormatter::Append0(str, (timeZoneDifferential%3600)/60, 2);
        }
        else
        {
            str += '-';
            NumberFormatter::Append0(str, -timeZoneDifferential/3600, 2);
            str += ':';
            NumberFormatter::Append0(str, (-timeZoneDifferential%3600)/60, 2);
        }
    }
    else 
    {
        str += 'Z';
    }
}

void DateTimeFormatter::TZDRFC(std::string& str, int timeZoneDifferential)
{
    if (timeZoneDifferential != UTC)
    {
        if (timeZoneDifferential >= 0)
        {
            str += '+';
            NumberFormatter::Append0(str, timeZoneDifferential/3600, 2);
            NumberFormatter::Append0(str, (timeZoneDifferential%3600)/60, 2);
        }
        else
        {
            str += '-';
            NumberFormatter::Append0(str, -timeZoneDifferential/3600, 2);
            NumberFormatter::Append0(str, (-timeZoneDifferential%3600)/60, 2);
        }		
    }
    else 
    {
        str += "GMT";
    }
}
