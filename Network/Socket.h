//////////////////////////////////////////////////////////////////////////
// Socket.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef Socket_INCLUDED
#define Socket_INCLUDED

#include "Types.h"
#include "SocketDefs.h"
#include "IPAddress.h"
#include "SocketAddress.h"
#include "Timespan.h"
#include "ErrorCodes.h"
#include <vector>

// Initialize WinSock in constructor; clean WinSock in destructor.
// Use this class before any further WinSock actions. 
class SocketAutoInit
{
public:
    SocketAutoInit()
    {
#ifdef _WIN32
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        int err = ::WSAStartup(wVersionRequested, &wsaData);
        if(err != 0)
        {
            // Failed to initialize WinSock
        }
#endif
    }

    ~SocketAutoInit()
    {
#ifdef _WIN32
        ::WSACleanup();
#endif
    }
};

// Socket is the common base class for TCP and UDP sockets
// It provides operations common to all socket types.
class Socket
{
public:
    // The mode argument to poll() and select().
    enum SelectMode
    {
        SELECT_READ = 1, SELECT_WRITE = 2, SELECT_ERROR = 4
    };

    // Creates an uninitialized socket.
    // SOCK_STREAM for TCP
    // SOCK_DGRAM for UDP
    Socket(int socketType = SOCK_STREAM);

    // Creates an initialized while unconnected socket.
    // The socket will be created for the given address family.
    explicit Socket(IPAddress::IPFamily family, int socketType = SOCK_STREAM);

    // Destroys the Socket
    virtual ~Socket();

    // Attach to a native socket.
    bool Attach(SOCKET_t sockfd);

    // Closes the socket.
    void Close();

    // Initializes the underlying native socket.
    virtual ErrorCode Init(int af = AF_INET);

    // Get the next completed connection from the
    // socket's completed connection queue.
    // If the queue is empty, waits until a connection request completes.
    // Returns a new TCP socket for the connection with the client.
    // The client socket's address is returned in clientAddr.
    virtual bool Accept(SocketAddress& clientAddr, Socket* clientSock);

    // Initializes the socket and establishes a connection to the TCP server at the given address.
    // Can also be used for UDP sockets. In this case, no
    // connection is established. Instead, incoming and outgoing
    // packets are restricted to the specified address.
    virtual ErrorCode Connect(const SocketAddress& address);

    // Initializes the socket, sets the socket timeout and
    // establishes a connection to the TCP server at the given address.
    virtual ErrorCode Connect(const SocketAddress& address, const Timespan& timeout);

    // Initializes the socket and establishes a connection to
    // the TCP server at the given address.
    // Prior to opening the connection the socket is set to nonblocking mode.
    virtual ErrorCode ConnectNB(const SocketAddress& address);

    // Initializes and binds a local address to the socket.
    // This is usually only done when establishing a server socket.
    // TCP clients should not bind a socket to specific address.
    // If reuseAddress is true, sets the SO_REUSEADDR socket option.
    virtual ErrorCode Bind(const SocketAddress& address, bool reuseAddress = false);

    // Puts the socket into listening state.
    // The socket becomes a passive socket that can accept incoming connection requests.
    // The backlog argument specifies the max number of connections that can be queued for this socket.
    virtual ErrorCode Listen(int backlog = 64);

    // Shuts down the receiving part of the socket connection.
    virtual ErrorCode ShutdownReceive();

    // Shuts down the sending part of the socket connection.
    virtual ErrorCode ShutdownSend();

    // Shuts down both the receiving and the sending part of the socket connection.
    virtual ErrorCode Shutdown();

    // Sends the contents of the given buffer through the socket.
    // Returns the number of bytes sent, which may be less than the number of bytes specified.
    // Certain socket implementations may also return a negative value denoting a certain condition.
    virtual int SendBytes(const void* buffer, int length, int flags = 0);

    // Sends the contents of the given buffer through the stream socket.
    // Returns the number of bytes sent, which may be less than the number of bytes specified.
    virtual int SendData(const void* buffer, int length, int flags = 0);

    // Receives data from the socket and stores it
    // in buffer. Up to length bytes are received.
    // Returns the number of bytes received.
    // Certain socket implementations may also return a negative
    // value denoting a certain condition.
    virtual int ReceiveBytes(void* buffer, int length, int flags = 0);

    // Read characters from socket until a newline is encountered.
    virtual std::string ReadLine(int maxLength);
    virtual int ReadLine(void* buffer, int maxLength);

    // Sends the contents of the given buffer through the socket to the given address.
    // Returns the number of bytes sent, which may be less than the number of bytes specified.
    virtual int SendTo(const void* buffer, int length,
            const SocketAddress& address, int flags = 0);

    // Receives data from the socket and stores it in buffer.
    // Up to length bytes are received. Stores the address of the sender in address.
    // Returns the number of bytes received.
    virtual int ReceiveFrom(void* buffer, int length, SocketAddress& address,
            int flags = 0);

    // Sends one byte of urgent data through the socket.
    // The data is sent with the MSG_OOB flag.
    // The preferred way for a socket to receive urgent data is by enabling the SO_OOBINLINE option.
    virtual int SendUrgent(unsigned char data);

    // Determines the status of one or more sockets, using a call to select().
    // ReadList contains the list of sockets which should be checked for readability.
    // WriteList contains the list of sockets which should be checked for write-ability.
    // ExceptList contains a list of sockets which should be checked for a pending error.
    // Returns the number of sockets ready.
    // After return, 
    //   * readList contains those sockets ready for reading,
    //   * writeList contains those sockets ready for writing,
    //   * exceptList contains those sockets with a pending error.
    // If the total number of sockets passed in readList, writeList and
    // exceptList is zero, select() will return immediately and the return value will be 0.
    // If one of the sockets passed to select() is closed while
    // select() runs, select will return immediately. However,
    // the closed socket will not be included in any list.
    // In this case, the return value may be greater than the sum of all sockets in all list.
    int Select(std::vector<Socket>& readList, std::vector<Socket>& writeList,
            std::vector<Socket>& exceptList, const Timespan& timeout);

    // Determines the status of the socket, using a call to select().
    // The mode argument is constructed by combining the values of the SelectMode enumeration.
    // Returns true if the next operation corresponding to mode will not block, false otherwise.
    bool Poll(const Timespan& timeout, int mode);

    // Returns the number of bytes available that can be read without causing the socket to block.
    int PeekAvailableRead();

    // Sets the size of the send buffer.
    ErrorCode SetSendBufferSize(int size);

    // Returns the size of the send buffer.
    // The returned value may be different than the value previously set with setSendBufferSize(),
    // as the system is free to adjust the value.
    int GetSendBufferSize();

    // Sets the size of the receive buffer.
    ErrorCode SetReceiveBufferSize(int size);

    // Returns the size of the receive buffer.
    // The returned value may be different than the value previously set with setReceiveBufferSize(),
    // as the system is free to adjust the value.
    int GetReceiveBufferSize();

    // Sets the send timeout for the socket.
    ErrorCode SetSendTimeout(const Timespan& timeout);

    // Returns the send timeout for the socket.
    // The returned timeout may be different than the timeout previously set with setSendTimeout(),
    // as the system is free to adjust the value.
    Timespan GetSendTimeout();

    // Sets the receive timeout for the socket.
    // On systems that do not support SO_RCVTIMEO, a workaround using poll() is provided.
    ErrorCode SetReceiveTimeout(const Timespan& timeout);

    // Returns the receive timeout for the socket.
    // The returned timeout may be different than the timeout previously set with getReceiveTimeout(),
    // as the system is free to adjust the value.
    Timespan GetReceiveTimeout();

    // Sets the socket option specified by level and option to the given integer value.
    ErrorCode SetOption(int level, int option, int value);

    // Sets the socket option specified by level and option to the given integer value.
    ErrorCode SetOption(int level, int option, unsigned int value);

    // Sets the socket option specified by level and option to the given integer value.
    ErrorCode SetOption(int level, int option, unsigned char value);

    // Sets the socket option specified by level and option to the given time value.
    ErrorCode SetOption(int level, int option, const Timespan& value);

    // Sets the socket option specified by level and option to the given time value.
    ErrorCode SetOption(int level, int option, const IPAddress& value);

    ErrorCode SetRawOption(int level, int option, const void* value,
            SOCKET_LENGTH_t length);

    // Returns the value of the socket option specified by level and option.
    ErrorCode GetOption(int level, int option, int& value);

    // Returns the value of the socket option specified by level and option.
    ErrorCode GetOption(int level, int option, unsigned int& value);

    // Returns the value of the socket option specified by level and option.
    ErrorCode GetOption(int level, int option, unsigned char& value);

    // Returns the value of the socket option  specified by level and option.
    ErrorCode GetOption(int level, int option, Timespan& value);

    // Returns the value of the socket option  specified by level and option.
    ErrorCode GetOption(int level, int option, IPAddress& value);

    ErrorCode GetRawOption(int level, int option, void* value,
            SOCKET_LENGTH_t& length);

    // Sets the value of the SO_LINGER socket option.
    ErrorCode SetLinger(bool on, int seconds);

    // Returns the value of the SO_LINGER socket option.
    void GetLinger(bool& on, int& seconds);

    // Sets the value of the TCP_NODELAY socket option.
    ErrorCode SetNoDelay(bool flag);

    // Returns the value of the TCP_NODELAY socket option.
    bool GetNoDelay();

    // Sets the value of the SO_KEEPALIVE socket option.
    ErrorCode SetKeepAlive(bool flag);

    // Returns the value of the SO_KEEPALIVE socket option.
    bool GetKeepAlive();

    // Sets the value of the SO_REUSEADDR socket option.
    ErrorCode SetReuseAddress(bool flag);

    // Returns the value of the SO_REUSEADDR socket option.
    bool GetReuseAddress();

    // Sets the value of the SO_REUSEPORT socket option.
    // Does nothing if the socket implementation does not support SO_REUSEPORT.
    ErrorCode SetReusePort(bool flag);

    // Returns the value of the SO_REUSEPORT socket option.
    // Returns false if the socket implementation does not support SO_REUSEPORT.
    bool GetReusePort() const;

    // Sets the value of the SO_OOBINLINE socket option.
    ErrorCode SetOOBInline(bool flag);

    // Returns the value of the SO_OOBINLINE socket option.
    bool GetOOBInline();

    // Sets the socket in blocking mode if flag is true,
    // disables blocking mode if flag is false.
    void SetBlocking(bool flag);

    // Returns the blocking mode of the socket.
    // This method will only work if the blocking modes of 
    // the socket are changed via the setBlocking method!
    bool GetBlocking() const;

    // Set the socket to blocking
    int SetBlocking();
    // Set the socket to non blocking
    int SetNonblocking();

    // Returns the IP address and port number of the socket.
    SocketAddress GetAddress();

    // Returns the IP address and port number of the peer socket.
    SocketAddress GetPeerAddress();

    // Returns the value of the SO_ERROR socket option.
    int GetSocketError();

    // Returns the socket descriptor for this socket.
    SOCKET_t Sockfd() const;

    // A wrapper for the ioctl system call.
    ErrorCode ioctl(int request, int& arg);

    // A wrapper for the ioctl system call.
    ErrorCode ioctl(int request, void* arg);

    ErrorCode GetErrorCode() const
    {
        return mErrorCode;
    }

    void SetErrorCode(ErrorCode errCode)
    {
        mErrorCode = errCode;
    }

    // Returns the last error code.
    int LastError() const
    {
        return errno;
    }

    ErrorCode HandleError();
    ErrorCode HandleError(const std::string& arg);
    ErrorCode HandleError(int code);
    ErrorCode HandleError(int code, const std::string& arg);

protected:
    // Creates the underlying native socket.
    // The first argument, af, specifies the address family
    // used by the socket, which should be either AF_INET or AF_INET6.
    // The second argument, type, specifies the type of the
    // socket, which can be one of SOCK_STREAM, SOCK_DGRAM or SOCK_RAW.
    // The third argument, proto, is normally set to 0, except for raw sockets.
    ErrorCode InitSocket(int af, int type, int proto = 0);

protected:
    int mSockType;
    SOCKET_t mSockfd;
    bool mIsBlocking;

    ErrorCode mErrorCode;
};

#endif // Socket_INCLUDED
