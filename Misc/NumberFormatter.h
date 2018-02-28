//////////////////////////////////////////////////////////////////////////
// NumberFormatter.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef NumberFormatter_INCLUDED
#define NumberFormatter_INCLUDED

#include "Types.h"
#include <string>

// The NumberFormatter class provides static methods for formatting numeric values into strings.
class NumberFormatter
{
public:
    // Formats an integer value in decimal notation.
    static std::string Format(int value);

    // Formats an integer value in decimal notation,
    // right justified in a field having at least the specified width.
    static std::string Format(int value, int width);

    // Formats an integer value in decimal notation, 
    // right justified and zero-padded in a field having at least the specified width.
    static std::string Format0(int value, int width);

    // Formats an int value in hexadecimal notation.
    // The value is treated as unsigned.
    static std::string FormatHex(int value);

    // Formats a int value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    // The value is treated as unsigned.
    static std::string FormatHex(int value, int width);

    // Formats an unsigned int value in decimal notation.
    static std::string Format(unsigned int value);

    // Formats an unsigned long int in decimal notation,
    // right justified in a field having at least the specified width.
    static std::string Format(unsigned int value, int width);

    // Formats an unsigned int value in decimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    static std::string Format0(unsigned int value, int width);

    // Formats an unsigned int value in hexadecimal notation.
    static std::string FormatHex(unsigned int value);

    // Formats a int value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    static std::string FormatHex(unsigned int value, int width);

#if defined(_WIN64) || defined(__LP64__)
    // Formats a 64-bit integer value in decimal notation.
    static std::string Format(Int64 value);

    // Formats a 64-bit integer value in decimal notation,
    // right justified in a field having at least the specified width.
    static std::string Format(Int64 value, int width);

    // Formats a 64-bit integer value in decimal notation, 
    // right justified and zero-padded in a field having at least  the specified width.
    static std::string Format0(Int64 value, int width);

    // Formats a 64-bit integer value in hexadecimal notation.
    // The value is treated as unsigned.
    static std::string FormatHex(Int64 value);

    // Formats a 64-bit integer value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    // The value is treated as unsigned.
    static std::string FormatHex(Int64 value, int width);

    // Formats an unsigned 64-bit integer value in decimal notation.
    static std::string Format(UInt64 value);

    // Formats an unsigned 64-bit integer value in decimal notation,
    // right justified in a field having at least the specified width.
    static std::string Format(UInt64 value, int width);

    // Formats an unsigned 64-bit integer value in decimal notation, 
    // right justified and zero-padded in a field having at least the specified width.
    static std::string Format0(UInt64 value, int width);

    // Formats a 64-bit integer value in hexadecimal notation.
    static std::string FormatHex(UInt64 value);

    // Formats a 64-bit integer value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    static std::string FormatHex(UInt64 value, int width);
#endif

    // Formats a float value in decimal floating-point notation,
    // according to std::printf's %g format with a precision of 8 fractional digits.
    static std::string Format(float value);

    // Formats a double value in decimal floating-point notation,
    // according to std::printf's %g format with a precision of 16 fractional digits.
    static std::string Format(double value);

    // Formats a double value in decimal floating-point notation,
    // according to std::printf's %f format with the given precision.
    static std::string Format(double value, int precision);

    // Formats a double value in decimal floating-point notation,
    // right justified in a field of the specified width,
    // with the number of fractional digits given in precision.
    static std::string Format(double value, int width, int precision);

    // Formats a pointer in an eight (32-bit architectures) or
    // sixteen (64-bit architectures) characters wide field in hexadecimal notation.
    static std::string Format(const void* ptr);

    // Formats an integer value in decimal notation.
    static void Append(std::string& str, int value);

    // Formats an integer value in decimal notation,
    // right justified in a field having at leas the specified width.
    static void Append(std::string& str, int value, int width);

    // Formats an integer value in decimal notation, 
    // right justified and zero-padded in a field having at least the specified width.
    static void Append0(std::string& str, int value, int width);

    // Formats an int value in hexadecimal notation.
    // The value is treated as unsigned.
    static void AppendHex(std::string& str, int value);

    // Formats a int value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    // The value is treated as unsigned.
    static void AppendHex(std::string& str, int value, int width);

    // Formats an unsigned int value in decimal notation.
    static void Append(std::string& str, unsigned int value);

    // Formats an unsigned long int in decimal notation,
    // right justified in a field having at least the specified width.
    static void Append(std::string& str, unsigned int value, int width);

    // Formats an unsigned int value in decimal notation, 
    // right justified and zero-padded in a field having at least the specified width.
    static void Append0(std::string& str, unsigned int value, int width);

    // Formats an unsigned int value in hexadecimal notation.
    static void AppendHex(std::string& str, unsigned int value);

    // Formats a int value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    static void AppendHex(std::string& str, unsigned int value, int width);

#if defined(_WIN64) || defined(__LP64__)
    // Formats a 64-bit integer value in decimal notation.
    static void Append(std::string& str, Int64 value);

    // Formats a 64-bit integer value in decimal notation,
    // right justified in a field having at least the specified width.
    static void Append(std::string& str, Int64 value, int width);

    // Formats a 64-bit integer value in decimal notation, 
    // right justified and zero-padded in a field having at least the specified width.
    static void Append0(std::string& str, Int64 value, int width);

    // Formats a 64-bit integer value in hexadecimal notation.
    // The value is treated as unsigned.
    static void AppendHex(std::string& str, Int64 value);

    // Formats a 64-bit integer value in hexadecimal notation,
    // right justified and zero-padded in a field having at least the specified width.
    // The value is treated as unsigned.
    static void AppendHex(std::string& str, Int64 value, int width);

    // Formats an unsigned 64-bit integer value in decimal notation.
    static void Append(std::string& str, UInt64 value);

    // Formats an unsigned 64-bit integer value in decimal notation,
    // right justified in a field having at least the specified width.
    static void Append(std::string& str, UInt64 value, int width);

    // Formats an unsigned 64-bit integer value in decimal notation, 
    // right justified and zero-padded in a field having at least the specified width.
    static void Append0(std::string& str, UInt64 value, int width);

    // Formats a 64-bit integer value in hexadecimal notation.
    static void AppendHex(std::string& str, UInt64 value);

    // Formats a 64-bit integer value in hexadecimal notation,
    // right justified and zero-padded in a field having at leas the specified width.
    static void AppendHex(std::string& str, UInt64 value, int width);
#endif

    // Formats a float value in decimal floating-point notation,
    // according to std::printf's %g format with a precision of 8 fractional digits.
    static void Append(std::string& str, float value);

    // Formats a double value in decimal floating-point notation,
    // according to std::printf's %g format with a precision of 16 fractional digits.
    static void Append(std::string& str, double value);

    // Formats a double value in decimal floating-point notation,
    // according to std::printf's %f format with the given precision.
    static void Append(std::string& str, double value, int precision);

    // Formats a double value in decimal floating-point notation,
    // right justified in a field of the specified width,
    // with the number of fractional digits given in precision.
    static void Append(std::string& str, double value, int width, int precision);

    // Formats a pointer in an eight (32-bit architectures) or
    // sixteen (64-bit architectures) characters wide field in hexadecimal notation.
    static void Append(std::string& str, const void* ptr);
};

inline std::string NumberFormatter::Format(int value)
{
    std::string result;
    Append(result, value);
    return result;
}

inline std::string NumberFormatter::Format(int value, int width)
{
    std::string result;
    Append(result, value, width);
    return result;
}

inline std::string NumberFormatter::Format0(int value, int width)
{
    std::string result;
    Append0(result, value, width);
    return result;
}

inline std::string NumberFormatter::FormatHex(int value)
{
    std::string result;
    AppendHex(result, value);
    return result;
}

inline std::string NumberFormatter::FormatHex(int value, int width)
{
    std::string result;
    AppendHex(result, value, width);
    return result;
}

inline std::string NumberFormatter::Format(unsigned int value)
{
    std::string result;
    Append(result, value);
    return result;
}

inline std::string NumberFormatter::Format(unsigned int value, int width)
{
    std::string result;
    Append(result, value, width);
    return result;
}

inline std::string NumberFormatter::Format0(unsigned int value, int width)
{
    std::string result;
    Append0(result, value, width);
    return result;
}

inline std::string NumberFormatter::FormatHex(unsigned int value)
{
    std::string result;
    AppendHex(result, value);
    return result;
}

inline std::string NumberFormatter::FormatHex(unsigned int value, int width)
{
    std::string result;
    AppendHex(result, value, width);
    return result;
}

#if defined(_WIN64) || defined(__LP64__)
inline std::string NumberFormatter::Format(Int64 value)
{
    std::string result;
    Append(result, value);
    return result;
}

inline std::string NumberFormatter::Format(Int64 value, int width)
{
    std::string result;
    Append(result, value, width);
    return result;
}

inline std::string NumberFormatter::Format0(Int64 value, int width)
{
    std::string result;
    Append0(result, value, width);
    return result;
}

inline std::string NumberFormatter::FormatHex(Int64 value)
{
    std::string result;
    AppendHex(result, value);
    return result;
}

inline std::string NumberFormatter::FormatHex(Int64 value, int width)
{
    std::string result;
    AppendHex(result, value, width);
    return result;
}

inline std::string NumberFormatter::Format(UInt64 value)
{
    std::string result;
    Append(result, value);
    return result;
}

inline std::string NumberFormatter::Format(UInt64 value, int width)
{
    std::string result;
    Append(result, value, width);
    return result;
}

inline std::string NumberFormatter::Format0(UInt64 value, int width)
{
    std::string result;
    Append0(result, value, width);
    return result;
}

inline std::string NumberFormatter::FormatHex(UInt64 value)
{
    std::string result;
    AppendHex(result, value);
    return result;
}

inline std::string NumberFormatter::FormatHex(UInt64 value, int width)
{
    std::string result;
    AppendHex(result, value, width);
    return result;
}
#endif

inline std::string NumberFormatter::Format(float value)
{
    std::string result;
    Append(result, value);
    return result;
}

inline std::string NumberFormatter::Format(double value)
{
    std::string result;
    Append(result, value);
    return result;
}

inline std::string NumberFormatter::Format(double value, int precision)
{
    std::string result;
    Append(result, value, precision);
    return result;
}

inline std::string NumberFormatter::Format(double value, int width,
        int precision)
{
    std::string result;
    Append(result, value, width, precision);
    return result;
}

inline std::string NumberFormatter::Format(const void* ptr)
{
    std::string result;
    Append(result, ptr);
    return result;
}

#endif // NumberFormatter_INCLUDED
