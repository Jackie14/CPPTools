//////////////////////////////////////////////////////////////////////////
// DateTimeFormatter.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef DateTimeFormatter_INCLUDED
#define DateTimeFormatter_INCLUDED

#include "DateTime.h"
#include <string>

class Timestamp;
class Timespan;

// Definition of date/time formats and various
class DateTimeFormat
{
public:
    // The date/time format defined in the ISO 8601 standard.
    // Examples:
    //   2005-01-01T12:00:00+01:00
    //   2005-01-01T11:00:00Z
    static const std::string ISO8601_FORMAT;

    // The date/time format defined in RFC 822 (obsoleted by RFC 1123).
    // Examples:
    //   Sat, 1 Jan 05 12:00:00 +0100
    //   Sat, 1 Jan 05 11:00:00 GMT
    static const std::string RFC822_FORMAT;

    // The date/time format defined in RFC 1123 (obsoletes RFC 822).
    // Examples:
    //   Sat, 1 Jan 2005 12:00:00 +0100
    //   Sat, 1 Jan 2005 11:00:00 GMT
    static const std::string RFC1123_FORMAT;

    // The date/time format defined in the HTTP specification (RFC 2616),
    // which is basically a variant of RFC 1036 with a zero-padded day field.
    // Examples:
    //   Sat, 01 Jan 2005 12:00:00 +0100
    //   Sat, 01 Jan 2005 11:00:00 GMT
    static const std::string HTTP_FORMAT;

    // The date/time format defined in RFC 850 (obsoleted by RFC 1036).
    // Examples:
    //   Saturday, 1-Jan-05 12:00:00 +0100
    //   Saturday, 1-Jan-05 11:00:00 GMT
    static const std::string RFC850_FORMAT;

    // The date/time format defined in RFC 1036 (obsoletes RFC 850).
    // Examples:
    //   Saturday, 1 Jan 05 12:00:00 +0100
    //   Saturday, 1 Jan 05 11:00:00 GMT
    static const std::string RFC1036_FORMAT;

    // The date/time format produced by the ANSI C asctime() function.
    // Example:
    //   Sat Jan  1 12:00:00 2005
    static const std::string ASCTIME_FORMAT;

    // A simple, sortable date/time format.
    // Example:
    //   2005-01-01 12:00:00
    static const std::string SORTABLE_FORMAT;

    // English names of week days (Sunday, Monday, Tuesday, ...).
    static const std::string WEEKDAY_NAMES[7];

    // English names of months (January, February, ...).
    static const std::string MONTH_NAMES[12];
};

// This class converts dates and times into strings, 
class DateTimeFormatter
{
public:
    enum
    {
        // Special value for timeZoneDifferential denoting UTC.
        UTC = 0xFFFF
    };

    // Formats the given timestamp according to the given format.
    // The format string is used as a template to format the date and
    // is copied character by character except for the following special characters,
    // which are replaced by the corresponding value.
    //   %w - abbreviated weekday (Mon, Tue, ...)
    //   %W - full weekday (Monday, Tuesday, ...)
    //   %b - abbreviated month (Jan, Feb, ...)
    //   %B - full month (January, February, ...)
    //   %d - zero-padded day of month (01 .. 31)
    //   %e - day of month (1 .. 31)
    //   %f - space-padded day of month ( 1 .. 31)
    //   %m - zero-padded month (01 .. 12)
    //   %n - month (1 .. 12)
    //   %o - space-padded month ( 1 .. 12)
    //   %y - year without century (70)
    //   %Y - year with century (1970)
    //   %H - hour (00 .. 23)
    //   %h - hour (00 .. 12)
    //   %a - am/pm
    //   %A - AM/PM
    //   %M - minute (00 .. 59)
    //   %S - second (00 .. 59)
    //   %i - millisecond (000 .. 999)
    //   %c - centisecond (0 .. 9)
    //   %F - fractional seconds/microseconds (000000 - 999999)
    //   %z - time zone differential in ISO 8601 format (Z or +NN.NN).
    //   %Z - time zone differential in RFC format (GMT or +NNNN)
    //   %% - percent sign
    static std::string Format(const Timestamp& timestamp,
            const std::string& fmt, int timeZoneDifferential = UTC);

    // Formats the given date and time according to the given format.
    static std::string Format(const DateTime& dateTime,
            const std::string& fmt, int timeZoneDifferential = UTC);

    // Formats the given timespan according to the given format.
    // The format string is used as a template to format the date and
    // is copied character by character except for the following special characters,
    // which are replaced by the corresponding value.
    //   %d - days
    //   %H - hours	 (00 .. 23)
    //   %h - total hours (0 .. n)
    //   %M - minutes (00 .. 59)
    //   %m - total minutes (0 .. n)
    //   %S - seconds (00 .. 59)
    //   %s - total seconds (0 .. n)
    //   %i - milliseconds (000 .. 999)
    //   %c - centisecond (0 .. 9)
    //   %F - fractional seconds/microseconds (000000 - 999999)
    //   %% - percent sign
    static std::string Format(const Timespan& timespan,
            const std::string& fmt = "%dd %H:%M:%S.%i");

    // Formats the given timestamp according to the given format and appends it to str.
    static void Append(std::string& str, const Timestamp& timestamp,
            const std::string& fmt, int timeZoneDifferential = UTC);

    // Formats the given date and time according to the given format and appends it to str.
    static void Append(std::string& str, const DateTime& dateTime,
            const std::string& fmt, int timeZoneDifferential = UTC);

    // Formats the given timespan according to the given format and appends it to str.
    static void Append(std::string& str, const Timespan& timespan,
            const std::string& fmt = "%dd %H:%M:%S.%i");

    // Formats the given timezone differential in ISO format.
    // If timeZoneDifferential is UTC, "Z" is returned,
    // otherwise, +HH.MM (or -HH.MM) is returned.
    static std::string TZDISO(int timeZoneDifferential);

    // Formats the given timezone differential in RFC format.
    // If timeZoneDifferential is UTC, "GMT" is returned,
    // otherwise ++HHMM (or -HHMM) is returned.
    static std::string TZDRFC(int timeZoneDifferential);

    // Formats the given timezone differential in ISO format
    // and appends it to the given string.
    // If timeZoneDifferential is UTC, "Z" is returned,
    // otherwise, +HH.MM (or -HH.MM) is returned.
    static void TZDISO(std::string& str, int timeZoneDifferential);

    // Formats the given timezone differential in RFC format
    // and appends it to the given string.
    // If timeZoneDifferential is UTC, "GMT" is returned,
    // otherwise ++HHMM (or -HHMM) is returned.
    static void TZDRFC(std::string& str, int timeZoneDifferential);
};

inline std::string DateTimeFormatter::Format(const Timestamp& timestamp,
        const std::string& fmt, int timeZoneDifferential)
{
    DateTime dateTime(timestamp);
    return Format(dateTime, fmt, timeZoneDifferential);
}

inline std::string DateTimeFormatter::Format(const DateTime& dateTime,
        const std::string& fmt, int timeZoneDifferential)
{
    std::string result;
    result.reserve(64);
    Append(result, dateTime, fmt, timeZoneDifferential);
    return result;
}

inline std::string DateTimeFormatter::Format(const Timespan& timespan,
        const std::string& fmt)
{
    std::string result;
    result.reserve(32);
    Append(result, timespan, fmt);
    return result;
}

inline void DateTimeFormatter::Append(std::string& str, const Timestamp& timestamp,
        const std::string& fmt, int timeZoneDifferential)
{
    DateTime dateTime(timestamp);
    Append(str, dateTime, fmt, timeZoneDifferential);
}

inline std::string DateTimeFormatter::TZDISO(int timeZoneDifferential)
{
    std::string result;
    result.reserve(8);
    TZDISO(result, timeZoneDifferential);
    return result;
}

inline std::string DateTimeFormatter::TZDRFC(int timeZoneDifferential)
{
    std::string result;
    result.reserve(8);
    TZDRFC(result, timeZoneDifferential);
    return result;
}

#endif // DateTimeFormatter_INCLUDED
