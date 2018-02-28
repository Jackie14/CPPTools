//////////////////////////////////////////////////////////////////////////
// MD5Hash.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef MD5Hash_INCLUDED
#define MD5Hash_INCLUDED

#include "HashBase.h"

// Data structure for MD5 (Message Digest) computation 
typedef struct
{
    unsigned int number[2]; // Number of _bits_ handled mod 2^64 
    unsigned int scratchBuffer[4]; // Scratch buffer 
    unsigned char inputBuffer[64]; // Input buffer
    unsigned char digest[16]; // Actual digest after MD5Final call 
} MD5Context;

//////////////////////////////////////////////////////////////////////////
class MD5Hash : public HashBase
{
public:
    MD5Hash();
    ~MD5Hash();

    virtual void Update(const unsigned char *data, unsigned int size);
    virtual void Finish();

    MD5Context GetResult() const;
    virtual std::string ToString() const;

protected:
    virtual void Init(unsigned int pseudoRandomNumber = 0);
    static void Transform(unsigned int *buf, unsigned int *in);

private:
    MD5Context mResult; 
};

#endif // MD5Hash_INCLUDED
