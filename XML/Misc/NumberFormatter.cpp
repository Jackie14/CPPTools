//////////////////////////////////////////////////////////////////////////
// NumberFormatter.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "NumberFormatter.h"
#include <cstdio>
#include <cctype>
#include <inttypes.h>

#if defined(_WIN64) || defined(__LP64__)
#define I64_FMT "l"
#else
#define I64_FMT "ll"
#endif

//////////////////////////////////////////////////////////////////////////
void NumberFormatter::Append(std::string& str, int value)
{
    char buffer[64];
    std::sprintf(buffer, "%d", value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, int value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%*d", width, value);
    str.append(buffer);
}

void NumberFormatter::Append0(std::string& str, int value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*d", width, value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, int value)
{
    char buffer[64];
    std::sprintf(buffer, "%X", value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, int value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*X", width, value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, unsigned int value)
{
    char buffer[64];
    std::sprintf(buffer, "%u", value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, unsigned int value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%*u", width, value);
    str.append(buffer);
}

void NumberFormatter::Append0(std::string& str, unsigned int value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*u", width, value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, unsigned int value)
{
    char buffer[64];
    std::sprintf(buffer, "%X", value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, unsigned int value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*X", width, value);
    str.append(buffer);
}

#if defined(_WIN64) || defined(__LP64__)

void NumberFormatter::Append(std::string& str, Int64 value)
{
    char buffer[64];
    std::sprintf(buffer, "%ld", value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, Int64 value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%*" I64_FMT "d", width, value);
    str.append(buffer);
}

void NumberFormatter::Append0(std::string& str, Int64 value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*" I64_FMT "d", width, value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, Int64 value)
{
    char buffer[64];
    std::sprintf(buffer, "%" I64_FMT "X", value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, Int64 value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*" I64_FMT "X", width, value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, UInt64 value)
{
    char buffer[64];
    std::sprintf(buffer, "%" I64_FMT "u", value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, UInt64 value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%*" I64_FMT "u", width, value);
    str.append(buffer);
}

void NumberFormatter::Append0(std::string& str, UInt64 value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*" I64_FMT "u", width, value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, UInt64 value)
{
    char buffer[64];
    std::sprintf(buffer, "%" I64_FMT "X", value);
    str.append(buffer);
}

void NumberFormatter::AppendHex(std::string& str, UInt64 value, int width)
{
    char buffer[64];
    std::sprintf(buffer, "%0*" I64_FMT "X", width, value);
    str.append(buffer);
}
#endif

void NumberFormatter::Append(std::string& str, float value)
{
    char buffer[64];
    std::sprintf(buffer, "%.*g", 8, (double) value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, double value)
{
    char buffer[64];
    std::sprintf(buffer, "%.*g", 16, value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, double value, int precision)
{
    char buffer[64];
    std::sprintf(buffer, "%.*f", precision, value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, double value, int width,
        int precision)
{
    char buffer[64];
    std::sprintf(buffer, "%*.*f", width, precision, value);
    str.append(buffer);
}

void NumberFormatter::Append(std::string& str, const void* ptr)
{
    char buffer[24];
#if defined(_WIN64) || defined(__LP64__)
    std::sprintf(buffer, "%016" I64_FMT "X", (UIntPtr) ptr);
#else
    std::sprintf(buffer, "%08lX", (UIntPtr) ptr);
#endif
    str.append(buffer);
}
