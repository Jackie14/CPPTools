//////////////////////////////////////////////////////////////////////////
// StringUtilities.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef StringUtilities_INCLUDED
#define StringUtilities_INCLUDED

#include <string>
#include <vector>

class StringUtilities
{
public:
    // Removes any new-line or carriage-return characters from the end of the string
    static int TrimNewLine(char *buffer, int length);
    static std::string TrimNewLine(const std::string& src);
    // Return sub-string of src, starting from left, with length
    static std::string ReduceLeft(const std::string& src, unsigned int length);
    // Return sub-string of src, reverse starting from right, with length
    static std::string ReduceRight(const std::string& src, unsigned int length);
    static char* StrToLower(char* str);
    static std::string StrToLower(const std::string& str);
    static char* StrToUpper(char* str);
    static std::string StrToUpper(const std::string& str);
    static std::string FormatString(const char *fmt, ...);
    static bool IsCRLF(const char* str, int length);
    static bool Replace(std::string& original, const std::string& from,
            const std::string& to);
    static bool ReplaceAll(std::string& original, const std::string& from,
            const std::string& to);
    // Take a host string and if there is a username/password part, strip it off.
    static void StripUsernamePassword(char* hostStr);
    // Take a host string and if there is a port part,
    // strip it off and set proper port variable
    static int StripPort(char* host);
    // For example, remove www. from www.example.com
    static std::string RemoveLeadingSubstr(const std::string& src,
            const std::string& subStr);
    static int CharToHex(unsigned char asciiByte);
    static unsigned char HexToChar(char hex);
    static void EncodeDomainNameToHex(const char* src, char* dest);

    static std::string Trim(const std::string& src, const std::string& toTrim = " ");
    static std::string TrimLeft(const std::string& src, const std::string& toTrim = "");
    static std::string TrimRight(const std::string& src, const std::string& toTrim = "");
    static bool SplitString(const std::string& src, const std::string& delims,
            bool trimSpace, std::vector<std::string>& result);
    // Read the first line from buffer
    static std::string ReadLineFromBuffer(const char* buffer);
    static bool IsAsciiString(const std::string &str);
};

#endif // StringUtilities_INCLUDED
