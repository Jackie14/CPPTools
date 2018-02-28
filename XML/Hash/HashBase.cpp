//////////////////////////////////////////////////////////////////////////
// HashBase.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "HashBase.h"
#include <stdio.h>

#define BufferSize 1024

HashBase::HashBase()
{
}

HashBase::~HashBase()
{
}

void HashBase::Init()
{
}

void HashBase::Reset()
{
    Init(); 
}

// hash a file
bool HashBase::HashFile(const char *fileName)
{
    Reset(); 

    FILE *file;

    if((file = fopen(fileName, "rb")) == NULL)
    {
        return false; // fail
    } 

    int lenRead = 0;
    unsigned char buffer[BufferSize];
    while((lenRead = fread(buffer, 1, BufferSize, file)))
    {
        Update(buffer, lenRead);
    }
    fclose(file);
    Finish();

    return true; // success
}

std::string HashBase::ToString() const
{
    return ""; 
}

// hash a string
void HashBase::HashString(const char *string)
{
    Reset(); 

    unsigned int len = strlen(string);

    Update((unsigned char *)string, len);
    Finish();
}

