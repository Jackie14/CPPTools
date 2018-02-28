//////////////////////////////////////////////////////////////////////////
// AutoBuffer.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef AutoBuffer_INCLUDED
#define AutoBuffer_INCLUDED

#include <cstddef>

// A very simple buffer class that allocates a buffer of
// a given type and size in the constructor and
// deallocates the buffer in the destructor.
template<class T>
class AutoBuffer
{
public:
    // Creates and allocates the Buffer.
    AutoBuffer(std::size_t size) :
        mSize(size), mPtr(new T[size])
    {
    }

    // Destroys the Buffer.
    ~AutoBuffer()
    {
        delete[] mPtr;
    }

    // Returns the size of the buffer.
    std::size_t size() const
    {
        return mSize;
    }

    // Returns a pointer to the beginning of the buffer.
    T* begin()
    {
        return mPtr;
    }

    // Returns a pointer to the beginning of the buffer.
    const T* begin() const
    {
        return mPtr;
    }

    // Returns a pointer to end of the buffer.
    T* end()
    {
        return mPtr + mSize;
    }

    // Returns a pointer to the end of the buffer.
    const T* end() const
    {
        return mPtr + mSize;
    }

    T& operator [](std::size_t index)
    {
        return mPtr[index];
    }

    const T& operator [](std::size_t index) const
    {
        return mPtr[index];
    }

private:
    AutoBuffer();
    AutoBuffer(const AutoBuffer&);
    AutoBuffer& operator =(const AutoBuffer&);

    std::size_t mSize;
    T* mPtr;
};

#endif // AutoBuffer_INCLUDED
