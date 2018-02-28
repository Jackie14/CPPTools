//////////////////////////////////////////////////////////////////////////
// StringUtilities.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "StringUtilities.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

// MIN and MAX.
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// Removes any new-line or carriage-return characters from the end of the string.
// "length" should be the number of characters in the buffer, not including
// the trailing NULL.
// Returns the number of characters removed from the end of the string.
// A negative return value indicates an error.
int StringUtilities::TrimNewLine(char *buffer, int length)
{
    int result;

    assert(buffer != NULL);
    assert(length > 0);

    // Make sure the arguments are valid
    if (buffer == NULL)
    {
        return -EFAULT;
    }
    if (length < 1)
    {
        return -ERANGE;
    }

    result = 0;

    --length;
    while (buffer[length] == '\r' || buffer[length] == '\n')
    {
        buffer[length] = '\0';
        result++;

        // Stop once we get to zero to prevent wrap-around
        if (length-- == 0)
        {
            break;
        }
    }

    return result;
}

std::string StringUtilities::TrimNewLine(const std::string& src)
{
    std::string ret;
    ret = Trim(src, "\n");
    ret = Trim(ret, "\r");

    return ret;
}

// Return sub-string of src, starting from left, with length
std::string StringUtilities::ReduceLeft(const std::string& src, unsigned int length)
{
    std::string result;
    if(src.size() < length)
    {
        return src;
    }

    unsigned int count = MAX(0, MIN(length, static_cast<unsigned int>(src.size())));
    result = src.substr(0, count);

    return result;
}

// Return sub-string of src, reverse starting from right, with length
std::string StringUtilities::ReduceRight(const std::string& src, unsigned int length)
{
    std::string result;
    if(src.size() < length)
    {
        return src;
    }

    unsigned int count = MAX(0, MIN(length, static_cast<unsigned int>(src.size())));
    result = src.substr(src.size() - count);

    return result;
}

char* StringUtilities::StrToLower(char* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    char* cp = str;
    while (*cp)
    {
        *cp = tolower(*cp);
        cp++;
    }

    return str;
}

std::string StringUtilities::StrToLower(const std::string& str)
{
    std::string result;
    size_t sz = str.size();
    if (sz <= 0)
    {
        return "";
    }

    char* szOld = new char[sz + 1];
    strcpy(szOld, str.c_str());
    szOld[sz] = '\0';
    result = StrToLower(szOld);
    delete[] szOld;

    return result;
}

char* StringUtilities::StrToUpper(char* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    char* cp = str;
    while (*cp)
    {
        *cp = toupper(*cp);
        cp++;
    }

    return str;
}

std::string StringUtilities::StrToUpper(const std::string& str)
{
    std::string result;
    size_t sz = str.size();
    if (sz <= 0)
    {
        return "";
    }

    char* szOld = new char[sz + 1];
    strcpy(szOld, str.c_str());
    szOld[sz] = '\0';
    result = StrToUpper(szOld);
    delete[] szOld;

    return result;
}

std::string StringUtilities::FormatString(const char *fmt, ...)
{
    // 8K buffer
    const int bufferSize = 1024 * 8;
    char buffer[bufferSize];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, bufferSize, fmt, args);
    va_end(args);

    std::string result = buffer;
    return result;
}

bool StringUtilities::IsCRLF(const char* str, int length)
{
    if (str == NULL)
    {
        return false;
    }
    if (strlen(str) <= 0)
    {
        return false;
    }

    if (((length == 1) && (str[0] == '\n')) || ((length == 2) && (str[0]
            == '\r') && (str[1] == '\n')))
    {
        return true;
    }

    return false;
}

bool StringUtilities::Replace(std::string& original, const std::string& from,
        const std::string& to)
{
    size_t startPos = original.find(from);
    if (startPos == std::string::npos)
    {
        return false;
    }
    original.replace(startPos, from.length(), to);
    return true;
}

bool StringUtilities::ReplaceAll(std::string& original,
        const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return false;
    }

    size_t startPos = 0;
    while ((startPos = original.find(from, startPos)) != std::string::npos)
    {
        original.replace(startPos, from.length(), to);
        // In case 'to' contains 'from', like replacing 'x' with 'yx'
        startPos += to.length();
    }
    return true;
}

// Take a host string and if there is a username/password part, strip it off.
void StringUtilities::StripUsernamePassword(char* hostStr)
{
    char *p;

    assert(hostStr);
    assert(strlen(hostStr) > 0);

    if ((p = strchr(hostStr, '@')) == NULL)
    {
        return;
    }

    // Move the pointer past the "@" and then copy from that point
    // until the NUL to the beginning of the host buffer.
    p++;
    while (*p)
    {
        *hostStr++ = *p++;
    }
    *hostStr = '\0';
}

// Take a host string and if there is a port part,
// strip it off and set proper port variable
int StringUtilities::StripPort(char* hostStr)
{
    char* ptr1 = strrchr(hostStr, ':');
    if (ptr1 == NULL)
    {
        return 0;
    }

    // for IPv6 style literals
    char* ptr2 = strchr(ptr1, ']');
    if (ptr2 != NULL)
    {
        return 0;
    }

    *ptr1++ = '\0';
    int port = 0;
    if (sscanf(ptr1, "%d", &port) != 1)
    {
        return 0;
    }

    return port;
}

std::string StringUtilities::RemoveLeadingSubstr(const std::string& src,
        const std::string& subStr)
{
    std::string result;
    if(src.size() <= 0)
    {
        return result;
    }
    if(src.size() < subStr.size())
    {
        return result;
    }

    size_t startPos = src.find(subStr);
    if (startPos != 0)
    {
        // Not starting with subStr
        return src;
    }

    result = ReduceRight(src, src.size() - subStr.size());

    return result;
}

int StringUtilities::CharToHex(unsigned char asciiByte)
{
    if (asciiByte <= 9)
    {
        return ('0' + asciiByte);
    }
    else if (asciiByte >= 10 && asciiByte <= 15)
    {
        return ('a' + (asciiByte - 10));
    }
    else
    {
        return -1;
    }
}

unsigned char StringUtilities::HexToChar(char hex)
{
    if (hex >= '0' && hex <= '9')
    {
        return (hex - '0');
    }
    else if (hex >= 'a' && hex <= 'f')
    {
        return (hex - 'a' + 10);
    }
    else
    {
        return -1;
    }
}

// Encode domain name to hex value
void StringUtilities::EncodeDomainNameToHex(const char* src, char* dest)
{
    if (dest == NULL || src == NULL)
    {
        return;
    }

    char c;
    while (*src != '\0')
    {
        if (*src != '.')
        {
            c = tolower((int)(*src));
            src++;
            *dest = CharToHex(c/16);
            dest++;
            *dest = CharToHex(c%16);
            dest++;
        }
        else
        {
            *dest = *src;
            dest++;
            src++;
        }
    }
}

std::string StringUtilities::TrimLeft(const std::string& src, const std::string& toTrim)
{
    std::string::size_type posBegin = src.find_first_not_of(toTrim);
    if (posBegin == std::string::npos)
    {
        return ""; // no content
    }

    std::string::size_type range = src.length() - posBegin + 1;

    return src.substr(posBegin, range);
}

std::string StringUtilities::TrimRight(const std::string& src, const std::string& toTrim)
{
    std::string::size_type posEnd = src.find_last_not_of(toTrim);
    if (posEnd == std::string::npos)
    {
        posEnd = src.length();
    }

    return src.substr(0, posEnd + 1);
}

std::string StringUtilities::Trim(const std::string& src, const std::string& toTrim)
{
    std::string::size_type posBegin = src.find_first_not_of(toTrim);
    if (posBegin == std::string::npos)
    {
        return ""; // no content
    }

    std::string::size_type posEnd = src.find_last_not_of(toTrim);
    if (posEnd == std::string::npos)
    {
        posEnd = src.length();
    }

    std::string::size_type range = posEnd - posBegin + 1;

    return src.substr(posBegin, range);
}

bool StringUtilities::SplitString(const std::string& src, const std::string& delims,
        bool trimSpace, std::vector<std::string>& result)
{
    if(src.size() <= 0 || delims.size() <= 0)
    {
        return false;
    }

    result.clear();

    std::string::size_type posBegin, posEnd;
    posBegin = src.find_first_not_of(delims);
    while (posBegin != std::string::npos)
    {
        posEnd = src.find_first_of(delims, posBegin);
        if (posEnd == std::string::npos)
        {
            posEnd = src.length();
        }

        std::string temp = src.substr(posBegin, posEnd - posBegin);
        if(trimSpace)
        {
            temp = Trim(temp);
        }

        if(temp.size() > 0)
        {
            result.push_back(temp);
        }

        posBegin = src.find_first_not_of(delims, posEnd);
    }

    return true;
}

std::string StringUtilities::ReadLineFromBuffer(const char* buffer)
{
    std::string ret;
    if(!buffer)
    {
        return "";
    }

    std::string::size_type pos = 0;
    int index = 0;
    std::string bufStr = buffer;
    do
    {
        pos = bufStr.find_first_of("\n");
        if(pos == std::string::npos)
        {
            return bufStr;
        }

        ret = bufStr.substr(0, pos);
        ret = TrimNewLine(ret);
        ret = Trim(ret);
        if(ret.size() <= 0)
        {
            // Avoid blank line
            index += pos;
            index++;
            bufStr = std::string(buffer + index);
            continue;
        }
        else
        {
            break;
        }
    }while(true);

    return ret;
}

bool StringUtilities::IsAsciiString(const std::string &str)
{
    int len = (int)str.length();
    for (int i = 0; i < len; i++)
    {
        if (!isascii(str.at(i)))
        {
            return false;
        }
    }

    return true;
}
