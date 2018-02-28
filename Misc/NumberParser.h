//////////////////////////////////////////////////////////////////////////
// NumberParser.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef NumberParser_INCLUDED
#define NumberParser_INCLUDED

#include "Types.h"
#include <string>

// Provide static methods for parsing numbers out of strings.
class NumberParser
{
public:
    // Parses an integer value in decimal notation from the given string.
    static int Parse(const std::string& s);

    // Parses an integer value in decimal notation from the given string.
    // Returns true if a valid integer has been found, false otherwise. 
    static bool Parse(const std::string& s, int& value);

    // Parses an unsigned integer value in decimal notation from the given string.
    static unsigned int ParseUnsignedInt(const std::string& s);

    // Parses an unsigned integer value in decimal notation from the given string.
    // Returns true if a valid integer has been found, false otherwise. 
    static bool ParseUnsignedInt(const std::string& s, unsigned int& value);

    // Parses an integer value in hexadecimal notation from the given string.
    static unsigned int ParseHex(const std::string& s);

    // Parses an unsigned integer value in hexadecimal notation from the given string.
    // Returns true if a valid integer has been found, false otherwise. 
    static bool ParseHex(const std::string& s, unsigned int& value);

    // Parses a 64-bit integer value in decimal notation from the given string.
    static Int64 Parse64(const std::string& s);

    // Parses a 64-bit integer value in decimal notation from the given string.
    // Returns true if a valid integer has been found, false otherwise. 
    static bool Parse64(const std::string& s, Int64& value);

    // Parses an unsigned 64-bit integer value in decimal notation from the given string.
    static UInt64 ParseUnsigned64(const std::string& s);

    // Parses an unsigned 64-bit integer value in decimal notation from the given string.
    // Returns true if a valid integer has been found, false otherwise. 
    static bool ParseUnsigned64(const std::string& s, UInt64& value);

    // Parses a 64 bit-integer value in hexadecimal notation from the given string.
    static UInt64 ParseHex64(const std::string& s);

    // Parses an unsigned 64-bit integer value in hexadecimal notation from the given string.
    // Returns true if a valid integer has been found, false otherwise. 
    static bool ParseHex64(const std::string& s, UInt64& value);

    // Parses a double value in decimal floating point notation from the given string.
    static double ParseFloat(const std::string& s);

    // Parses a double value in decimal floating point notation from the given string.
    // Returns true if a valid floating point number has been found, false otherwise.
    static bool ParseFloat(const std::string& s, double& value);
};

#endif // NumberParser_INCLUDED
