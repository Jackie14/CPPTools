//////////////////////////////////////////////////////////////////////////
// Types.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef Types_INCLUDED
#define Types_INCLUDED

// Unix/GCC
typedef char Int8;
typedef unsigned char UInt8;
typedef short Int16;
typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
typedef long IntPtr;
typedef unsigned long UIntPtr;
#if defined(__LP64__)
typedef long Int64;
typedef unsigned long UInt64;
#else
typedef long long Int64;
typedef unsigned long long UInt64;
#endif


#endif // Types_INCLUDED
