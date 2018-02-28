/*
 * Base64.cpp
 *
 */

#include "Base64.h"

void Base64::Encode(const std::vector<unsigned char>& plain, std::string& encoded)
{
    // Map number to Base64 char;
    static const char N2C[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    encoded.clear();

    int len = plain.size();
    int i = 0;

    for (; i + 2 < len; i += 3)
    {
        encoded += N2C[(plain[i] >> 2) & 0x3F];
        encoded += N2C[((plain[i] & 0x3) << 4) | ((int) (plain[i + 1] & 0xF0) >> 4)];
        encoded += N2C[((plain[i + 1] & 0xF) << 2) | ((int) (plain[i + 2] & 0xC0) >> 6)];
        encoded += N2C[plain[i + 2] & 0x3F];
    }

    if (i < len) 
    {
        encoded += N2C[(plain[i] >> 2) & 0x3F];

        if (i == (len - 1))
        {
            encoded += N2C[((plain[i] & 0x3) << 4)];
            encoded += '=';
        }
        else
        {
            encoded +=  N2C[((plain[i] & 0x3) << 4) | ((int) (plain[i + 1] & 0xF0) >> 4)];
            encoded +=  N2C[((plain[i + 1] & 0xF) << 2)];
        }
        encoded += '=';
    }

    return;
}


// Decode coded into plain.  Return false for invalid base64 encoded string
bool Base64::Decode(const std::string& encoded, std::vector<unsigned char>& plain)
{
    // Map char to base64 number;
    static const unsigned char C2N[256] =
    {
        // ASCII table
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    plain.clear();

    int len = encoded.length();
    int i = 0;

    for (; i + 3 < len; i += 4)
    {
        if (C2N[(unsigned char)encoded[i]] <= 63 &&
                C2N[(unsigned char)encoded[i + 1]] <= 63 &&
                C2N[(unsigned char)encoded[i + 2]] <= 63 &&
                C2N[(unsigned char)encoded[i + 3]] <= 63)
        {
            plain.push_back(C2N[(unsigned char)encoded[i]] << 2 | C2N[(unsigned char)encoded[i + 1]] >> 4);
            plain.push_back(C2N[(unsigned char)encoded[i + 1]] << 4 | C2N[(unsigned char)encoded[i + 2]] >> 2);
            plain.push_back(C2N[(unsigned char)encoded[i + 2]] << 6 | C2N[(unsigned char)encoded[i + 3]]);
        }
        else
        {
            break;
        }
    }

    // For the case, encoded string has no padding '='
    if (i == len)
    {
        return true;
    }

    // For the case, encoded string end with padding '=', like "xxx=" or "xx==", in which x is valid base64 char
    if (i + 4 == len &&
            C2N[(unsigned char)encoded[i]] <= 63 &&
            C2N[(unsigned char)encoded[i + 1]] <= 63 &&
            encoded[i + 3] == '=')
    {
        plain.push_back(C2N[(unsigned char)encoded[i]] << 2 | C2N[(unsigned char)encoded[i + 1]] >> 4);

        if (C2N[(unsigned char)encoded[i + 2]] <= 63) // xxx=
        {
            plain.push_back(C2N[(unsigned char)encoded[i + 1]] << 4 | C2N[(unsigned char)encoded[i + 2]] >> 2);
            return true;
        }
        else if (encoded[i + 2] == '=') // xx==
        {
            return true;
        }

    }

    // For other cases, return false, for format error
    return false;
}

