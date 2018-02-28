//////////////////////////////////////////////////////////////////////////
// Socket.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "Socket.h"
#include "Timestamp.h"
#include "Log.h"
#include "assert.h"
#include <algorithm>
#include <string.h>
#include <iostream>

Socket::Socket(int socketType)
{
    mSockType = socketType;
    mSockfd = INVALID_SOCKET_T;
    mIsBlocking = true;
    mErrorCode = ErrorOK;
}

Socket::Socket(IPAddress::IPFamily family, int socketType)
{
    mSockType = socketType;
    mSockfd = INVALID_SOCKET_T;
    mIsBlocking = true;
    mErrorCode = ErrorOK;

    if (family == IPAddress::IPv4)
    {
        Init(AF_INET);
    }
    else if (family == IPAddress::IPv6)
    {
        Init(AF_INET6);
    }
    else
    {
        LOG(LogError, "Invalid or unsupported address family");
        SetErrorCode(ErrorInvalidArgument);
    }
}

Socket::~Socket()
{
    Close();
}

bool Socket::Attach(SOCKET_t sockfd)
{
    if(sockfd <= 0)
    {
        return false;
    }

    mSockfd = sockfd;
    mIsBlocking = true;
    return true;
}

void Socket::Close()
{
    if (mSockfd != INVALID_SOCKET_T)
    {
        CLOSE_SOCKET(mSockfd);
        mSockfd = INVALID_SOCKET_T;
    }
}

bool Socket::Accept(SocketAddress& clientAddr, Socket* clientSock)
{
    if(!clientSock)
    {
        return false;
    }

    char buffer[SocketAddress::MAX_ADDRESS_LENGTH];
    sockaddr* pSA = reinterpret_cast<sockaddr*>(buffer);
    SOCKET_LENGTH_t saLen = sizeof(buffer);
    SOCKET_t sd;
    do
    {
        sd = accept(mSockfd, pSA, &saLen);
    }
    while (sd == INVALID_SOCKET_T && LastError() == SOCKET_ERROR_INTR);

    if (sd != INVALID_SOCKET_T)
    {
        bool hasError = false;
        clientAddr = SocketAddress(pSA, saLen, hasError);
        if(hasError || clientAddr.GetAddr() == NULL)
        {
            return false;
        }

        clientSock->Attach(sd);
        return true;
    }

    HandleError();
    return false;
}

ErrorCode Socket::Connect(const SocketAddress& address)
{
    ErrorCode ret = ErrorOK;
    if(address.GetAddr() == NULL)
    {
        return ErrorNetworkAddrNotAvailable;
    }

    if (mSockfd == INVALID_SOCKET_T)
    {
        ret = Init(address.GetAF());
        if(ret != ErrorOK)
        {
            return ret;
        }
    }

    int rc;
    do
    {
        rc = connect(mSockfd, address.GetAddr(), address.GetLength());
    }
    while (rc != 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc != 0)
    {
        ret = HandleError(address.ToString());
    }

    return ret;
}

ErrorCode Socket::Connect(const SocketAddress& address, const Timespan& timeout)
{
    ErrorCode ret = ErrorOK;
    if(address.GetAddr() == NULL)
    {
        return ErrorNetworkAddrNotAvailable;
    }

    if (mSockfd == INVALID_SOCKET_T)
    {
        Init(address.GetAF());
    }
    SetBlocking(false);
    try
    {
        int rc = connect(mSockfd, address.GetAddr(), address.GetLength());
        if (rc != 0)
        {
            if (LastError() != SOCKET_ERROR_INPROGRESS && LastError()
                    != SOCKET_ERROR_WOULDBLOCK)
            {
                ret = HandleError(address.ToString());
                return ret;
            }

            if (!Poll(timeout, SELECT_READ | SELECT_WRITE))
            {
                LOG(LogError, "Connect timed out: %s", address.ToString().c_str());
                SetErrorCode(ErrorNetworkTimeout);
                return ErrorNetworkTimeout;
            }

            int err = GetSocketError();
            if (err != 0)
            {
                ret = HandleError(err);
                return ret;
            }
        }
        else
        {
            LOG(LogDebug, "Socket connected");
        }
    }
    catch (...)
    {
        SetBlocking(true);
        LOG(LogError, "Socket exception");
        SetErrorCode(ErrorNetworkSocketException);
        return ErrorNetworkSocketException;
    }
    SetBlocking(true);

    return ret;
}

ErrorCode Socket::ConnectNB(const SocketAddress& address)
{
    ErrorCode ret = ErrorOK;
    if(address.GetAddr() == NULL)
    {
        return ErrorNetworkAddrNotAvailable;
    }

    if (mSockfd == INVALID_SOCKET_T)
    {
        Init(address.GetAF());
    }
    SetBlocking(false);
    int rc = connect(mSockfd, address.GetAddr(), address.GetLength());
    if (rc != 0)
    {
        if (LastError() != SOCKET_ERROR_INPROGRESS && LastError()
                != SOCKET_ERROR_WOULDBLOCK)
        {
            ret = HandleError(address.ToString());
            return ret;
        }
    }

    return ret;
}

ErrorCode Socket::Bind(const SocketAddress& address, bool reuseAddress)
{
    ErrorCode ret = ErrorOK;
    if(address.GetAddr() == NULL)
    {
        return ErrorNetworkAddrNotAvailable;
    }

    if (mSockfd == INVALID_SOCKET_T)
    {
        Init(address.GetAF());
    }
    if (reuseAddress)
    {
        SetReuseAddress(true);
        SetReusePort(true);
    }
    int rc = bind(mSockfd, address.GetAddr(), address.GetLength());
    if (rc != 0)
    {
        LOG(LogError, "Failed to bind socket: %s", address.ToString().c_str());
        ret = HandleError(address.ToString());
        return ret;
    }

    return ret;
}

ErrorCode Socket::Listen(int backlog)
{
    ErrorCode ret = ErrorOK;

    int rc = ::listen(mSockfd, backlog);
    if (rc != 0)
    {
        LOG(LogError, "Failed to listen socket. ");
        ret = HandleError();
    }

    return ret;
}

ErrorCode Socket::ShutdownReceive()
{
    ErrorCode ret = ErrorOK;

    int rc = shutdown(mSockfd, 0);
    if (rc != 0)
    {
        ret = HandleError();
    }

    return ret;
}

ErrorCode Socket::ShutdownSend()
{
    ErrorCode ret = ErrorOK;

    int rc = shutdown(mSockfd, 1);
    if (rc != 0)
    {
        ret = HandleError();
    }

    return ret;
}

ErrorCode Socket::Shutdown()
{
    ErrorCode ret = ErrorOK;

    int rc = shutdown(mSockfd, 2);
    if (rc != 0)
    {
        ret = HandleError();
    }

    return ret;
}

int Socket::SendBytes(const void* buffer, int length, int flags)
{
    int rc;
    do
    {
        rc = send(mSockfd, reinterpret_cast<const char*>(buffer), length,
                flags);
    }
    while (rc < 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc < 0)
    {
        HandleError();
    }

    return rc;
}

int Socket::SendData(const void* buffer, int length, int flags)
{
    const char* p = reinterpret_cast<const char*>(buffer);
    int remaining = length;
    int sent = 0;
    while (remaining > 0 && GetBlocking())
    {
        int n = SendBytes(p, remaining, flags);
        if (n <= 0)
        {
            return n;
        }
        p += n;
        remaining -= n;
        sent += n;
    }
    return sent;
}

int Socket::ReceiveBytes(void* buffer, int length, int flags)
{
    int rc;
    do
    {
        rc = recv(mSockfd, reinterpret_cast<char*>(buffer), length, flags);
    }
    while (rc < 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc < 0)
    {
        if (LastError() == SOCKET_ERROR_AGAIN || LastError()
                == SOCKET_ERROR_TIMEDOUT)
        {
            LOG(LogError, "Timeout when receive. ");
            SetErrorCode(ErrorNetworkTimeout);
            return -1;
        }
        else
        {
            HandleError();
        }
    }
    else if (rc == 0)
    {
        // Peer socket shutdown
        SocketAddress peer = GetPeerAddress();
    }
    else
    {
        LOG(LogDebug, "ReceiveBytes from: %d", mSockfd);
    }

    return rc;
}

// Read characters from socket until a newline is encountered.
std::string Socket::ReadLine(int maxLength)
{
    if(maxLength <= 0)
    {
        errno = EINVAL;
        return "";
    }

    std::string result;
    char* buffer = new char[maxLength];
    if(!buffer)
    {
        LOG(LogError, "Out of memory.");
        return "";
    }

    bzero(buffer, maxLength);
    int ret = ReadLine(buffer, maxLength);
    if(ret <= 0)
    {
        delete []buffer;
        return "";
    }

    result = buffer;
    delete []buffer;

    return result;
}

// Read characters from socket until a newline is encountered.
int Socket::ReadLine(void* buffer, int maxLength)
{
    if (maxLength <= 0 || buffer == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    char* szBuf = (char*) buffer;
    // Total bytes read so far
    int totalRead = 0;
    for (;;)
    {
        char ch;
        int numRead = read(mSockfd, &ch, 1);
        if (numRead == -1)
        {
            if (errno == EINTR)
            {
                // Interrupted, restart read()
                continue;
            }
            else
            {
                // Some other error
                return -1;
            }
        }
        else if (numRead == 0)
        {
            // EOF
            if (totalRead == 0)
            {
                // No bytes read; return 0
                return 0;
            }
            else
            {
                // Some bytes read; add '\0'
                break;
            }
        }
        else
        {
            // numRead must be 1 if we get here
            if (totalRead < maxLength - 1)
            {
                // Discard > (n - 1) bytes
                totalRead++;
                *szBuf++ = ch;
            }

            if (ch == '\n')
            {
                break;
            }
        }
    }

    *szBuf = '\0';
    return totalRead;
}

int Socket::SendTo(const void* buffer, int length,
        const SocketAddress& address, int flags)
{
    int rc;
    do
    {
        rc = sendto(mSockfd, reinterpret_cast<const char*>(buffer), length,
                flags, address.GetAddr(), address.GetLength());
    }
    while (rc < 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc < 0)
    {
        HandleError();
    }
    else
    {
        LOG(LogDebug, "SendTo");
    }

    return rc;
}

int Socket::ReceiveFrom(void* buffer, int length, SocketAddress& address,
        int flags)
{
    char abuffer[SocketAddress::MAX_ADDRESS_LENGTH];
    sockaddr* pSA = reinterpret_cast<sockaddr*>(abuffer);
    SOCKET_LENGTH_t saLen = sizeof(abuffer);
    int rc;
    do
    {
        rc = recvfrom(mSockfd, reinterpret_cast<char*>(buffer), length, flags,
                pSA, &saLen);
    }
    while (rc < 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc >= 0)
    {
        LOG(LogDebug, "ReceiveFrom");
        bool hasError = false;
        address = SocketAddress(pSA, saLen, hasError);
    }
    else
    {
        if (LastError() == SOCKET_ERROR_AGAIN || LastError()
                == SOCKET_ERROR_TIMEDOUT)
        {
            LOG(LogError, "Timeout when recvfrom. ");
            SetErrorCode(ErrorNetworkTimeout);
            return -1;
        }
        else
        {
            HandleError();
        }
    }
    return rc;
}

int Socket::SendUrgent(unsigned char data)
{
    int rc = send(mSockfd, reinterpret_cast<const char*>(&data), sizeof(data),
            MSG_OOB);
    if (rc < 0)
    {
        HandleError();
    }

    return rc;
}

int Socket::Select(std::vector<Socket>& readList, std::vector<Socket>& writeList,
        std::vector<Socket>& exceptList, const Timespan& timeout)
{
    fd_set fdRead;
    fd_set fdWrite;
    fd_set fdExcept;
    int nfd = 0;

    FD_ZERO(&fdRead);
    for (std::vector<Socket>::const_iterator it = readList.begin();
            it != readList.end(); ++it)
    {
        SOCKET_t fd = it->Sockfd();
        if (fd != INVALID_SOCKET_T)
        {
            if (int(fd) > nfd)
            {
                nfd = int(fd);
            }
            FD_SET(fd, &fdRead);
        }
    }

    FD_ZERO(&fdWrite);
    for (std::vector<Socket>::const_iterator it = writeList.begin(); it
            != writeList.end(); ++it)
    {
        SOCKET_t fd = it->Sockfd();
        if (fd != INVALID_SOCKET_T)
        {
            if (int(fd) > nfd)
            {
                nfd = int(fd);
            }
            FD_SET(fd, &fdWrite);
        }
    }

    FD_ZERO(&fdExcept);
    for (std::vector<Socket>::const_iterator it = exceptList.begin(); it
            != exceptList.end(); ++it)
    {
        SOCKET_t fd = it->Sockfd();
        if (fd != INVALID_SOCKET_T)
        {
            if (int(fd) > nfd)
            {
                nfd = int(fd);
            }
            FD_SET(fd, &fdExcept);
        }
    }

    if (nfd == 0)
    {
        return 0;
    }

    Timespan remainingTime(timeout);
    int rc;
    do
    {
        timeval tv;
        tv.tv_sec = (long)remainingTime.GetTotalSeconds();
        tv.tv_usec = (long)remainingTime.GetUseconds();
        Timestamp start;
        rc = select(nfd + 1, &fdRead, &fdWrite, &fdExcept, &tv);
        if (rc < 0 && LastError() == SOCKET_ERROR_INTR)
        {
            Timestamp end;
            Timespan waited = end - start;
            if (waited < remainingTime)
            {
                remainingTime -= waited;
            }
            else
            {
                remainingTime = 0;
            }
        }
    }
    while (rc < 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc < 0)
    {
        HandleError();
        return rc;
    }

    std::vector<Socket> readyReadList;
    for (std::vector<Socket>::const_iterator it = readList.begin();
            it != readList.end(); ++it)
    {
        SOCKET_t fd = it->Sockfd();
        if (fd != INVALID_SOCKET_T)
        {
            if (FD_ISSET(fd, &fdRead))
            {
                readyReadList.push_back(*it);
            }
        }
    }

    std::swap(readList, readyReadList);
    std::vector<Socket> readyWriteList;
    for (std::vector<Socket>::const_iterator it = writeList.begin();
            it != writeList.end(); ++it)
    {
        SOCKET_t fd = it->Sockfd();
        if (fd != INVALID_SOCKET_T)
        {
            if (FD_ISSET(fd, &fdWrite))
            {
                readyWriteList.push_back(*it);
            }
        }
    }

    std::swap(writeList, readyWriteList);
    std::vector<Socket> readyExceptList;
    for (std::vector<Socket>::const_iterator it = exceptList.begin();
            it != exceptList.end(); ++it)
    {
        SOCKET_t fd = it->Sockfd();
        if (fd != INVALID_SOCKET_T)
        {
            if (FD_ISSET(fd, &fdExcept))
            {
                readyExceptList.push_back(*it);
            }
        }
    }

    std::swap(exceptList, readyExceptList);
    return rc;
}

// Determines the status of the socket, using a call to select().
// The mode argument is constructed by combining the values
// of the SelectMode enumeration.
// Returns true if the next operation corresponding to
// mode will not block, false otherwise.
bool Socket::Poll(const Timespan& timeout, int mode)
{
    fd_set fdRead;
    fd_set fdWrite;
    fd_set fdExcept;
    FD_ZERO(&fdRead);
    FD_ZERO(&fdWrite);
    FD_ZERO(&fdExcept);
    if (mode & SELECT_READ)
    {
        FD_SET(mSockfd, &fdRead);
    }
    if (mode & SELECT_WRITE)
    {
        FD_SET(mSockfd, &fdWrite);
    }
    if (mode & SELECT_ERROR)
    {
        FD_SET(mSockfd, &fdExcept);
    }

    Timespan remainingTime(timeout);
    int rc;
    do
    {
        timeval tv;
        tv.tv_sec = (long)remainingTime.GetTotalSeconds();
        tv.tv_usec = (long)remainingTime.GetUseconds();
        Timestamp start;
        rc = select(int(mSockfd) + 1, &fdRead, &fdWrite, &fdExcept, &tv);
        if (rc < 0 && LastError() == SOCKET_ERROR_INTR)
        {
            Timestamp end;
            Timespan waited = end - start;
            if (waited < remainingTime)
            {
                remainingTime -= waited;
            }
            else
            {
                remainingTime = 0;
            }
        }
    }
    while (rc < 0 && LastError() == SOCKET_ERROR_INTR);

    if (rc < 0)
    {
        HandleError();
    }
    return (rc > 0);
}

// Returns the number of bytes available that can be read
// without causing the socket to block.
int Socket::PeekAvailableRead()
{
    int result;
    ioctl(FIONREAD, result);
    return result;
}

// Sets the size of the send buffer.
ErrorCode Socket::SetSendBufferSize(int size)
{
    return SetOption(SOL_SOCKET, SO_SNDBUF, size);
}

// Returns the size of the send buffer.
// The returned value may be different than the
// value previously set with setSendBufferSize(),
// as the system is free to adjust the value.
int Socket::GetSendBufferSize()
{
    int result;
    GetOption(SOL_SOCKET, SO_SNDBUF, result);
    return result;
}

// Sets the size of the receive buffer.
ErrorCode Socket::SetReceiveBufferSize(int size)
{
    return SetOption(SOL_SOCKET, SO_RCVBUF, size);
}

// Returns the size of the receive buffer.
// The returned value may be different than the
// value previously set with setReceiveBufferSize(),
// as the system is free to adjust the value.
int Socket::GetReceiveBufferSize()
{
    int result;
    GetOption(SOL_SOCKET, SO_RCVBUF, result);
    return result;
}

// Sets the send timeout for the socket.
ErrorCode Socket::SetSendTimeout(const Timespan& timeout)
{
    return SetOption(SOL_SOCKET, SO_SNDTIMEO, timeout);
}

// Returns the send timeout for the socket.
// The returned timeout may be different than the
// timeout previously set with setSendTimeout(),
// as the system is free to adjust the value.
Timespan Socket::GetSendTimeout()
{
    Timespan result;
    GetOption(SOL_SOCKET, SO_SNDTIMEO, result);
    return result;
}

// Sets the receive timeout for the socket.
// On systems that do not support SO_RCVTIMEO, a
// workaround using poll() is provided.
ErrorCode Socket::SetReceiveTimeout(const Timespan& timeout)
{
    return SetOption(SOL_SOCKET, SO_RCVTIMEO, timeout);
}

// Returns the receive timeout for the socket.
// The returned timeout may be different than the
// timeout previously set with getReceiveTimeout(),
// as the system is free to adjust the value.
Timespan Socket::GetReceiveTimeout()
{
    Timespan result;
    GetOption(SOL_SOCKET, SO_RCVTIMEO, result);
    return result;
}

// Sets the socket option specified by level and option
// to the given integer value.
ErrorCode Socket::SetOption(int level, int option, int value)
{
    return SetRawOption(level, option, &value, sizeof(value));
}

// Sets the socket option specified by level and option
// to the given integer value.
ErrorCode Socket::SetOption(int level, int option, unsigned int value)
{
    return SetRawOption(level, option, &value, sizeof(value));
}

// Sets the socket option specified by level and option
// to the given integer value.
ErrorCode Socket::SetOption(int level, int option, unsigned char value)
{
    return SetRawOption(level, option, &value, sizeof(value));
}

// Sets the socket option specified by level and option
// to the given time value.
ErrorCode Socket::SetOption(int level, int option, const Timespan& value)
{
    timeval tv;
    tv.tv_sec = (long)value.GetTotalSeconds();
    tv.tv_usec = (long)value.GetUseconds();

    return SetRawOption(level, option, &tv, sizeof(tv));
}

// Sets the socket option specified by level and option
// to the given time value.
ErrorCode Socket::SetOption(int level, int option, const IPAddress& value)
{
    return SetRawOption(level, option, value.GetAddr(), value.GetLength());
}

ErrorCode Socket::SetRawOption(int level, int option, const void* value,
        SOCKET_LENGTH_t length)
{
    ErrorCode ret = ErrorOK;
    int rc = setsockopt(mSockfd, level, option,
            reinterpret_cast<const char*>(value), length);
    if (rc == -1)
    {
        ret = HandleError();
    }

    return ret;
}

// Returns the value of the socket option 
// specified by level and option.
ErrorCode Socket::GetOption(int level, int option, int& value)
{
    SOCKET_LENGTH_t len = sizeof(value);
    return GetRawOption(level, option, &value, len);
}

// Returns the value of the socket option 
// specified by level and option.
ErrorCode Socket::GetOption(int level, int option, unsigned int& value)
{
    SOCKET_LENGTH_t len = sizeof(value);
    return GetRawOption(level, option, &value, len);
}

// Returns the value of the socket option 
// specified by level and option.
ErrorCode Socket::GetOption(int level, int option, unsigned char& value)
{
    SOCKET_LENGTH_t len = sizeof(value);
    return GetRawOption(level, option, &value, len);
}

// Returns the value of the socket option 
// specified by level and option.
ErrorCode Socket::GetOption(int level, int option, Timespan& value)
{
    timeval tv;
    SOCKET_LENGTH_t len = sizeof(tv);
    ErrorCode ret = GetRawOption(level, option, &tv, len);
    value.Assign(tv.tv_sec, tv.tv_usec);

    return ret;
}

// Returns the value of the socket option 
// specified by level and option.
ErrorCode Socket::GetOption(int level, int option, IPAddress& value)
{
    char buffer[IPAddress::MAX_ADDRESS_LENGTH];
    SOCKET_LENGTH_t len = sizeof(buffer);
    ErrorCode ret = GetRawOption(level, option, buffer, len);
    bool hasError = false;
    value = IPAddress(buffer, len, hasError);

    return ret;
}

ErrorCode Socket::GetRawOption(int level, int option, void* value,
        SOCKET_LENGTH_t& length)
{
    ErrorCode ret = ErrorOK;
    int rc = getsockopt(mSockfd, level, option,
            reinterpret_cast<char*>(value), &length);
    if (rc == -1)
    {
        ret = HandleError();
    }

    return ret;
}

// Sets the value of the SO_LINGER socket option.
ErrorCode Socket::SetLinger(bool on, int seconds)
{
    linger l;
    l.l_onoff = on ? 1 : 0;
    l.l_linger = seconds;
    return SetRawOption(SOL_SOCKET, SO_LINGER, &l, sizeof(l));
}

// Returns the value of the SO_LINGER socket option.
void Socket::GetLinger(bool& on, int& seconds)
{
    linger l;
    SOCKET_LENGTH_t len = sizeof(l);
    GetRawOption(SOL_SOCKET, SO_LINGER, &l, len);
    on = l.l_onoff != 0;
    seconds = l.l_linger;
}

// Sets the value of the TCP_NODELAY socket option.
ErrorCode Socket::SetNoDelay(bool flag)
{
    int value = flag ? 1 : 0;
    return SetOption(IPPROTO_TCP, TCP_NODELAY, value);
}

// Returns the value of the TCP_NODELAY socket option.
bool Socket::GetNoDelay()
{
    int value(0);
    GetOption(IPPROTO_TCP, TCP_NODELAY, value);
    return value != 0;
}

// Sets the value of the SO_KEEPALIVE socket option.
ErrorCode Socket::SetKeepAlive(bool flag)
{
    int value = flag ? 1 : 0;
    return SetOption(SOL_SOCKET, SO_KEEPALIVE, value);
}

// Returns the value of the SO_KEEPALIVE socket option.
bool Socket::GetKeepAlive()
{
    int value(0);
    GetOption(SOL_SOCKET, SO_KEEPALIVE, value);
    return value != 0;
}

// Sets the value of the SO_REUSEADDR socket option.
ErrorCode Socket::SetReuseAddress(bool flag)
{
    int value = flag ? 1 : 0;
    return SetOption(SOL_SOCKET, SO_REUSEADDR, value);
}

// Returns the value of the SO_REUSEADDR socket option.
bool Socket::GetReuseAddress()
{
    int value(0);
    GetOption(SOL_SOCKET, SO_REUSEADDR, value);
    return value != 0;
}

// Sets the value of the SO_REUSEPORT socket option.
// Does nothing if the socket implementation does not support SO_REUSEPORT.
ErrorCode Socket::SetReusePort(bool flag)
{
    return ErrorOK;
}

// Returns the value of the SO_REUSEPORT socket option.
// Returns false if the socket implementation does not support SO_REUSEPORT.
bool Socket::GetReusePort() const
{
    return false;
}

// Sets the value of the SO_OOBINLINE socket option.
ErrorCode Socket::SetOOBInline(bool flag)
{
    int value = flag ? 1 : 0;
    return SetOption(SOL_SOCKET, SO_OOBINLINE, value);
}

// Returns the value of the SO_OOBINLINE socket option.
bool Socket::GetOOBInline()
{
    int value(0);
    GetOption(SOL_SOCKET, SO_OOBINLINE, value);
    return value != 0;
}

// Sets the socket in blocking mode if flag is true,
// disables blocking mode if flag is false.
void Socket::SetBlocking(bool flag)
{
    int arg = flag ? 0 : 1;
    ioctl(FIONBIO, arg);
    mIsBlocking = flag;
}

// Returns the blocking mode of the socket.
// This method will only work if the blocking modes of 
// the socket are changed via the setBlocking method!
bool Socket::GetBlocking() const
{
    return mIsBlocking;
}

// Set the socket to blocking
int Socket::SetBlocking()
{
    assert(mSockfd >= 0);

    mIsBlocking = true;
    int flags = fcntl(mSockfd, F_GETFL, 0);
    return fcntl(mSockfd, F_SETFL, flags & ~O_NONBLOCK);
}

// Set the socket to non blocking
int Socket::SetNonblocking()
{
    assert(mSockfd >= 0);

    mIsBlocking = false;
    int flags = fcntl(mSockfd, F_GETFL, 0);
    return fcntl(mSockfd, F_SETFL, flags | O_NONBLOCK);
}

// Returns the IP address and port number of the socket.
SocketAddress Socket::GetAddress()
{
    char buffer[SocketAddress::MAX_ADDRESS_LENGTH];
    sockaddr* pSA = reinterpret_cast<sockaddr*>(buffer);
    SOCKET_LENGTH_t saLen = sizeof(buffer);
    int rc = getsockname(mSockfd, pSA, &saLen);
    if (rc == 0)
    {
        bool hasError = false;
        return SocketAddress(pSA, saLen, hasError);
    }
    else
    {
        HandleError();
        return SocketAddress();
    }
}

// Returns the IP address and port number of the peer socket.
SocketAddress Socket::GetPeerAddress()
{
    char buffer[SocketAddress::MAX_ADDRESS_LENGTH];
    sockaddr* pSA = reinterpret_cast<sockaddr*>(buffer);
    SOCKET_LENGTH_t saLen = sizeof(buffer);
    int rc = getpeername(mSockfd, pSA, &saLen);
    if (rc == 0)
    {
        bool hasError = false;
        return SocketAddress(pSA, saLen, hasError);
    }
    else
    {
        HandleError();
        return SocketAddress();
    }
}

int Socket::GetSocketError()
{
    int result(0);
    GetOption(SOL_SOCKET, SO_ERROR, result);
    return result;
}

SOCKET_t Socket::Sockfd() const
{
    return mSockfd;
}

ErrorCode Socket::Init(int af)
{
    return InitSocket(af, mSockType);
}

ErrorCode Socket::InitSocket(int af, int type, int proto)
{
    ErrorCode ret = ErrorOK;
    mSockfd = socket(af, type, proto);
    if (mSockfd == INVALID_SOCKET_T)
    {
        ret = HandleError();
    }

    return ret;
}

// A wrapper for the ioctl system call.
ErrorCode Socket::ioctl(int request, int& arg)
{
    ErrorCode ret = ErrorOK;
    int rc = ::ioctl(mSockfd, request, &arg);
    if (rc != 0)
    {
        ret = HandleError();
    }

    return ret;
}

// A wrapper for the ioctl system call.
ErrorCode Socket::ioctl(int request, void* arg)
{
    ErrorCode ret = ErrorOK;
    int rc = ::ioctl(mSockfd, request, arg);
    if (rc != 0)
    {
        ret = HandleError();
    }

    return ret;
}

ErrorCode Socket::HandleError()
{
    std::string empty;
    return HandleError(LastError(), empty);
}

ErrorCode Socket::HandleError(const std::string& arg)
{
    return HandleError(LastError(), arg);
}

ErrorCode Socket::HandleError(int code)
{
    std::string arg;
    return HandleError(code, arg);
}

// Trigger events
ErrorCode Socket::HandleError(int code, const std::string& arg)
{
    ErrorCode ret;

    switch (code)
    {
    case SOCKET_ERROR_SYSNOTREADY:
        LOG(LogError, "Net subsystem not ready");
        ret = ErrorNetworkSysNotReady;
        break;
    case SOCKET_ERROR_NOTINIT:
        LOG(LogError, "Net subsystem not initialized");
        ret = ErrorNetworkSysNotInit;
        break;
    case SOCKET_ERROR_INTR:
        LOG(LogError, "Interrupted");
        ret = ErrorNetworkInterrupted;
        break;
    case SOCKET_ERROR_ACCES:
        LOG(LogError, "Permission denied");
        ret = ErrorNetworkPermissionDenied;
        break;
    case SOCKET_ERROR_FAULT:
        LOG(LogError, "Bad address");
        ret = ErrorNetworkBadAddress;
        break;
    case SOCKET_ERROR_INVAL:
        LOG(LogError, "Invalid Argument");
        ret = ErrorInvalidArgument;
        break;
    case SOCKET_ERROR_MFILE:
        LOG(LogError, "Too many open files");
        ret = ErrorNetworkTooManyOpening;
        break;
    case SOCKET_ERROR_WOULDBLOCK:
        LOG(LogError, "Operation would block");
        ret = ErrorNetworkOperationBlock;
        break;
    case SOCKET_ERROR_INPROGRESS:
        LOG(LogError, "Operation now in progress");
        ret = ErrorNetworkOperationInProgress;
        break;
    case SOCKET_ERROR_ALREADY:
        LOG(LogError, "Operation already in progress");
        ret = ErrorNetworkOperationAlready;
        break;
    case SOCKET_ERROR_NOTSOCK:
        LOG(LogError, "Socket operation attempted on non-socket");
        ret = ErrorNetworkNonSocket;
        break;
    case SOCKET_ERROR_DESTADDRREQ:
        LOG(LogError, "Destination address required");
        ret = ErrorNetworkDestAddrRequired;
        break;
    case SOCKET_ERROR_MSGSIZE:
        LOG(LogError, "Message too long");
        ret = ErrorNetworkMessageTooLong;
        break;
    case SOCKET_ERROR_PROTOTYPE:
        LOG(LogError, "Wrong protocol type");
        ret = ErrorNetworkWrongProtocol;
        break;
    case SOCKET_ERROR_NOPROTOOPT:
        LOG(LogError, "Protocol not available");
        ret = ErrorNetworkProtocolNotAvailable;
        break;
    case SOCKET_ERROR_PROTONOSUPPORT:
        LOG(LogError, "Protocol not supported");
        ret = ErrorNetworkProtocolNotSupported;
        break;
    case SOCKET_ERROR_SOCKTNOSUPPORT:
        LOG(LogError, "Socket type not supported");
        ret = ErrorNetworkSocketNotSupported;
        break;
    case SOCKET_ERROR_NOTSUP:
        LOG(LogError, "Operation not supported");
        ret = ErrorNetworkOperationNotSupported;
        break;
    case SOCKET_ERROR_PFNOSUPPORT:
        LOG(LogError, "Protocol family not supported");
        ret = ErrorNetworkProtocolFamilyNotSupported;
        break;
    case SOCKET_ERROR_AFNOSUPPORT:
        LOG(LogError, "Address family not supported");
        ret = ErrorNetworkAddrFamilyNotSupported;
        break;
    case SOCKET_ERROR_ADDRINUSE:
        LOG(LogError, "Address already in use");
        ret = ErrorNetworkAddrInUse;
        break;
    case SOCKET_ERROR_ADDRNOTAVAIL:
        LOG(LogError, "Cannot assign requested address");
        ret = ErrorNetworkAddrNotAvailable;
        break;
    case SOCKET_ERROR_NETDOWN:
        LOG(LogError, "Network is down");
        ret = ErrorNetworkDown;
        break;
    case SOCKET_ERROR_NETUNREACH:
        LOG(LogError, "Network is unreachable");
        ret = ErrorNetworkUnreachable;
        break;
    case SOCKET_ERROR_NETRESET:
        LOG(LogError, "Network dropped connection on reset");
        ret = ErrorNetworkReset;
        break;
    case SOCKET_ERROR_CONNABORTED:
        LOG(LogError, "Connection Aborted");
        ret = ErrorNetworkConnectionAborted;
        break;
    case SOCKET_ERROR_CONNRESET:
        LOG(LogError, "Connection Reset");
        ret = ErrorNetworkConnectionReset;
        break;
    case SOCKET_ERROR_NOBUFS:
        LOG(LogError, "No buffer space available");
        ret = ErrorNetworkNoBufferSpace;
        break;
    case SOCKET_ERROR_ISCONN:
        LOG(LogError, "Socket is already connected");
        ret = ErrorNetworkSocketIsConnected;
        break;
    case SOCKET_ERROR_NOTCONN:
        LOG(LogError, "Socket is not connected");
        ret = ErrorNetworkSocketIsNotConnected;
        break;
    case SOCKET_ERROR_SHUTDOWN:
        LOG(LogError, "Cannot send after socket shutdown");
        ret = ErrorNetworkShutdown;
        break;
    case SOCKET_ERROR_TIMEDOUT:
        LOG(LogError, "Timeout");
        ret = ErrorNetworkTimeout;
        break;
    case SOCKET_ERROR_CONNREFUSED:
        LOG(LogError, "Connection Refused");
        ret = ErrorNetworkConnectionRefused;
        break;
    case SOCKET_ERROR_HOSTDOWN:
        LOG(LogError, "Host is down");
        ret = ErrorNetworkHostDown;
        break;
    case SOCKET_ERROR_HOSTUNREACH:
        LOG(LogError, "No route to host");
        ret = ErrorNetworkHostUnreachable;
        break;
    default:
        LOG(LogError, arg.c_str());
        ret = ErrorNetwork;
        break;
    }

    mErrorCode = ret;
    return ret;
}
