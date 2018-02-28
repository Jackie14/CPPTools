/*
 * Base64.h
 *
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <string>
#include <vector>

class Base64
{
public:
    static void Encode(const std::vector<unsigned char>& plain, std::string& encoded);
    static bool Decode(const std::string& encoded, std::vector<unsigned char>& plain);
};


#endif /* BASE64_H_ */
