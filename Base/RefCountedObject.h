//////////////////////////////////////////////////////////////////////////
// RefCountedObject.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef RefCountedObject_INCLUDED
#define RefCountedObject_INCLUDED

#include "AtomicCounter.h"

// A base class for objects that employ reference counting based garbage collection.
class RefCountedObject
{
public:
    // The initial reference count is one.
    RefCountedObject() :
        mCounter(1)
    {
    }

    // Increments the object's reference count.
    void Duplicate() const
    {
        ++mCounter;
    }

    // Decrements the object's reference count
    // and deletes the object if the count reaches zero.
    void Release() const
    {
        if (--mCounter == 0)
        {
            delete this;
        }
    }

    // Returns the reference count.
    int ReferenceCount() const
    {
        return mCounter.Value();
    }

protected:
    // Destroys the RefCountedObject.
    virtual ~RefCountedObject()
    {
    }

private:
    RefCountedObject(const RefCountedObject&);
    RefCountedObject& operator =(const RefCountedObject&);

    mutable AtomicCounter mCounter;
};

#endif // RefCountedObject_INCLUDED
