//////////////////////////////////////////////////////////////////////////
// SmartPtr.h
//
//////////////////////////////////////////////////////////////////////////

#ifndef SmartPtr_INCLUDED
#define SmartPtr_INCLUDED

#include "Process.h"

template<class T>
class smart_ptr
{
public:
    explicit smart_ptr(T* rhs = 0);
    smart_ptr(const smart_ptr<T>& rhs);
    template<class U> smart_ptr(const smart_ptr<U>& rhs);
    ~smart_ptr();
    
    smart_ptr<T>& operator =(const smart_ptr<T>& rhs);
    smart_ptr<T>& operator =(T* rhs);
    template<class U> smart_ptr<T>& operator =(const smart_ptr<U>& rhs);
    
    bool operator == (T* rhs)
    {
        return (ptr == rhs);
    }
    bool operator == (const smart_ptr<T>& rhs)
    {
        return (ptr == rhs.ptr);
    }
    bool operator != (T* rhs)
    {
        return (ptr != rhs);
    }
    
    T* operator ->() const;
    T& operator*() const;
    bool operator !() const; // test for null
    T* get() const;
    void release();

private:
    void inc();
    void dec();
    
    T* ptr;
    int* ref;
};

template<class T>
inline smart_ptr<T>::smart_ptr(T* rhs) : ptr(rhs)
{
    ref = new int(1);
}

template<class T>
inline smart_ptr<T>::smart_ptr(const smart_ptr<T>& rhs)
{
    ptr = rhs.ptr;
    ref = rhs.ref;
    inc();
}

template<class T>
template<class U>
inline smart_ptr<T>::smart_ptr(const smart_ptr<U>& rhs)
{
    ptr = rhs.ptr;
    ref = rhs.ref;
    inc();
}

template<class T>
inline smart_ptr<T>::~smart_ptr()
{
    dec();
}

template<class T>
inline void smart_ptr<T>::inc()
{
    Process::AtomInc(ref);
}

template<class T>
inline void smart_ptr<T>::dec()
{
    if (Process::AtomDec(ref) == 0)
    {
        if (ptr)
            delete ptr;
        if (ref)
            delete ref;
    }
}

template<class T>
inline void smart_ptr<T>::release()
{
    dec();
    ref = new int(1);
    ptr = 0;
}

template<class T>
inline smart_ptr<T>& smart_ptr<T>::operator =(const smart_ptr<T>& rhs)
{
    if (ptr != rhs.ptr)
    {
        dec();
        ptr = rhs.ptr;
        ref = rhs.ref;
        inc();
    }
    return *this;
}

template<class T>
inline smart_ptr<T>& smart_ptr<T>::operator =(T* rhs)
{
    dec();
    ptr = rhs;
    ref = new int(1);
    return *this;
}

template<class T>
template<class U>
inline smart_ptr<T>& smart_ptr<T>::operator =(const smart_ptr<U>& rhs)
{
    if (ptr != rhs.ptr)
    {
        dec();
        ptr = rhs.ptr;
        ref = rhs.ref;
        inc();
    }
    return *this;
}

template<class T>
inline T* smart_ptr<T>::operator ->() const
{
    return ptr;
}

template<class T>
inline T& smart_ptr<T>::operator*() const
{
    return *ptr;
}

template<class T>
inline bool smart_ptr<T>::operator !() const
{
    return (ptr == 0);
}

template<class T>
inline T* smart_ptr<T>::get() const
{
    return ptr;
}

#endif // SmartPtr_INCLUDED
