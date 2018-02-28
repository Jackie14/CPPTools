//////////////////////////////////////////////////////////////////////////
// CRC32Hash.h 
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef CRC32Hash_INCLUDED
#define CRC32Hash_INCLUDED

#include "HashBase.h"

class CRC32Hash : public HashBase
{
public:
    CRC32Hash();
    ~CRC32Hash();

    virtual void Update(const unsigned char *data, unsigned int size);
    virtual void Finish();

    unsigned int GetResult() const;
    virtual std::string ToString() const; 

protected:
    virtual void Init();

private:
    unsigned int mResult; 
};

#endif // CRC32Hash_INCLUDED
