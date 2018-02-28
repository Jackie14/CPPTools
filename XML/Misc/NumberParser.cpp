//////////////////////////////////////////////////////////////////////////
// NumberParser.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "NumberParser.h"
#include <cstdio>
#include <cctype>

#if defined(__LP64__)
#define I64_FMT "l"
#else
#define I64_FMT "ll"
#endif

int NumberParser::Parse(const std::string& s)
{
    int result;
    if (Parse(s, result))
    {
        return result;
    }
    else
    {
        return 0;
    }
}

bool NumberParser::Parse(const std::string& s, int& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%d%c", &value, &temp) == 1;
}

unsigned int NumberParser::ParseUnsignedInt(const std::string& s)
{
    unsigned int result;
    if (ParseUnsignedInt(s, result))
    {
        return result;
    }
    else
    {
        return 0;
    }
}

bool NumberParser::ParseUnsignedInt(const std::string& s, unsigned& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%u%c", &value, &temp) == 1;
}

unsigned int NumberParser::ParseHex(const std::string& s)
{
    unsigned int result;
    if (ParseHex(s, result))
    {
        return result;
    }
    else
    {
        return 0;
    }
}

bool NumberParser::ParseHex(const std::string& s, unsigned int& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%x%c", &value, &temp) == 1;
}

Int64 NumberParser::Parse64(const std::string& s)
{
    Int64 result;
    if (Parse64(s, result))
    {
        return result;
    }
    else
    {;
        return 0;
    }
}

bool NumberParser::Parse64(const std::string& s, Int64& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%" I64_FMT "d%c", &value, &temp) == 1;
}

UInt64 NumberParser::ParseUnsigned64(const std::string& s)
{
    UInt64 result;
    if (ParseUnsigned64(s, result))
    {
        return result;
    }
    else
    {
        return 0;
    }
}

bool NumberParser::ParseUnsigned64(const std::string& s, UInt64& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%" I64_FMT "u%c", &value, &temp) == 1;
}

UInt64 NumberParser::ParseHex64(const std::string& s)
{
    UInt64 result;
    if (ParseHex64(s, result))
    {
        return result;
    }
    else
    {
        return 0;
    }
}

bool NumberParser::ParseHex64(const std::string& s, UInt64& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%" I64_FMT "x%c", &value, &temp) == 1;
}

double NumberParser::ParseFloat(const std::string& s)
{
    double result;
    if (ParseFloat(s, result))
    {
        return result;
    }
    else
    {
        return 0;
    }
}

bool NumberParser::ParseFloat(const std::string& s, double& value)
{
    char temp;
    return std::sscanf(s.c_str(), "%lf%c", &value, &temp) == 1;
}

