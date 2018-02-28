//////////////////////////////////////////////////////////////////////////
// SocketAddress.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef SocketAddress_INCLUDED
#define SocketAddress_INCLUDED

#include "Types.h"
#include "SocketDefs.h"
#include "IPAddress.h"

class IPAddress;
class SocketAddressImpl;
// This class represents an internet (IP) endpoint/socket address.
// The address can belong either to the IPv4 or the IPv6 address family
// and consists of a host address and a port number.
class SocketAddress
{
public:
    // Creates a wild card (all zero) IPv4 SocketAddress.
    SocketAddress();

    // Creates a SocketAddress from an IP address and a port number.
    SocketAddress(const IPAddress& host, UInt16 port, bool& hasError);

    // Creates a SocketAddress from an IP address and a port number.
    // The IP address must either be a domain name, or it must
    // be in dotted decimal (IPv4) or hex string (IPv6) format.
    SocketAddress(const std::string& host, UInt16 port, bool& hasError);

    // Creates a SocketAddress from an IP address and a service name or port number.
    // The IP address must either be a domain name, or it must
    // be in dotted decimal (IPv4) or hex string (IPv6) format.
    // The given port must either be a decimal port number, or a service name.
    SocketAddress(const std::string& host, const std::string& port, bool& hasError);

    // Creates a SocketAddress from an IP address or host name and a port number/service name.
    // Host name/address and port number must be separated by a colon.
    // In case of an IPv6 address, the address part must be enclosed in brackets.
    // Examples:
    //     192.168.1.10:80 
    //     [::FFFF:192.168.1.120]:2040
    //     www.appinf.com:8080
    explicit SocketAddress(const std::string& hostAndPort, bool& hasError);

    // Creates a SocketAddress by copying another one.
    SocketAddress(const SocketAddress& addr);

    // Creates a SocketAddress from a native socket address.
    SocketAddress(const sockaddr* addr, SOCKET_LENGTH_t length, bool& hasError);

    // Destroys the SocketAddress.
    ~SocketAddress();

    // Assigns another SocketAddress.
    SocketAddress& operator =(const SocketAddress& addr);

    // Returns the host IP address.
    bool GetHost(IPAddress& ipAddr) const;

    // Returns the port number.
    UInt16 GetPort() const;

    // Returns the length of the internal native socket address.	
    SOCKET_LENGTH_t GetLength() const;

    // Returns a pointer to the internal native socket address.
    const sockaddr* GetAddr() const;

    // Returns the address family (AF_INET or AF_INET6) of the address.
    int GetAF() const;

    // Returns a string representation of the address.
    std::string ToString() const;

    // Returns the address family of the host's address.
    IPAddress::IPFamily GetFamily() const
    {
        IPAddress ipAddr;
        GetHost(ipAddr);
        return ipAddr.GetFamily();
    }

    // Maximum length in bytes of a socket address.
    enum
    {
        MAX_ADDRESS_LENGTH = sizeof(sockaddr_in6) //sizeof(struct sockaddr_in)
    };

protected:
    bool Init(const IPAddress& host, UInt16 port);
    bool Init(const std::string& host, UInt16 port);
    UInt16 ResolveService(const std::string& service);

private:
    SocketAddressImpl* mImpl;
};

#endif // SocketAddress_INCLUDED
