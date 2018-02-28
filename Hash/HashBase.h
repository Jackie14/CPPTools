//////////////////////////////////////////////////////////////////////////
// HashBase.h
//////////////////////////////////////////////////////////////////////////

#ifndef HashBase_INCLUDED
#define HashBase_INCLUDED

#include <string.h>
#include <iostream>

class HashBase
{
public:
    HashBase();
    virtual ~HashBase();

    virtual void Update(const unsigned char *data, unsigned int size) = 0;
    virtual void Finish() = 0;

    // hash a file
    virtual bool HashFile(const char *fileName); 
    // hash a string
    virtual void HashString(const char *string); 

    virtual void Reset();

    virtual std::string ToString() const; 

protected:
    virtual void Init();
};

#endif // HashBase_INCLUDED
