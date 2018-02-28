//////////////////////////////////////////////////////////////////////////
// IPAddress.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef IPAddress_INCLUDED
#define IPAddress_INCLUDED

#include <string>
#include "SocketDefs.h"

// This class represents an Internet (IP) host address.
// The address can belong either to the IPv4 or the IPv6 address family.
class IPAddressImpl;
class IPAddress
{
public:
    // Possible address families for IP addresses.
    enum IPFamily
    {
        IPv4, IPv6
    };

    // Creates a wild card (zero) IPv4 IPAddress.
    IPAddress();

    // Creates an IPAddress by copying another one.
    IPAddress(const IPAddress& addr);

    // Creates a wild card (zero) IPAddress for the given address family.
    explicit IPAddress(IPFamily family);

    // Creates an IPAddress from the string containing an IP address
    // in presentation format (dotted decimal for IPv4, hex string for IPv6).
    explicit IPAddress(const std::string& addr, bool& hasError);

    // Creates an IPAddress from the string containing an IP address
    // in presentation format (dotted decimal for IPv4, hex string for IPv6).
    IPAddress(const std::string& addr, IPFamily family, bool& hasError);

    // Creates an IPAddress from a native internet address.
    // A pointer to a in_addr or a in6_addr structure may be passed.
    IPAddress(const void* addr, SOCKET_LENGTH_t Length, bool& hasError);

    // Creates an IPAddress from a native internet address.
    // A pointer to a in_addr or a in6_addr structure may be passed.
    // Additionally, for an IPv6 address, a scope ID may be specified.
    // The scope ID will be ignored if an IPv4 address is specified.
    IPAddress(const void* addr, SOCKET_LENGTH_t length, unsigned int scope,
            bool& hasError);

    // Destroys the IPAddress.
    ~IPAddress();

    // Assigns an IPAddress.
    IPAddress& operator =(const IPAddress& addr);

    // Returns the address family (IPv4 or IPv6) of the address.
    IPFamily GetFamily() const;

    // Returns a string containing a representation of the address in presentation format.
    // For IPv4 addresses the result will be in dotted-decimal (d.d.d.d) notation.
    // Textual representation of IPv6 address is one of the following forms:
    // The preferred form is x:x:x:x:x:x:x:x, where the 'x's are the hexadecimal 
    // values of the eight 16-bit pieces of the address. This is the full form.
    // Example: 1080:0:0:0:8:600:200A:425C
    // It is not necessary to write the leading zeros in an individual field. 
    // However, there must be at least one numeral in every field, except as described below.
    // It is common for IPv6 addresses to contain long strings of zero bits. 
    // In order to make writing addresses containing zero bits easier, a special syntax is 
    // available to compress the zeros. The use of "::" indicates multiple groups of 16-bits of zeros. 
    // The "::" can only appear once in an address. The "::" can also be used to compress the leading 
    // and/or trailing zeros in an address. Example: 1080::8:600:200A:425C
    // For dealing with IPv4 compatible addresses in a mixed environment,
    // a special syntax is available: x:x:x:x:x:x:d.d.d.d, where the 'x's are the 
    // hexadecimal values of the six high-order 16-bit pieces of the address, 
    // and the 'd's are the decimal values of the four low-order 8-bit pieces of the 
    // standard IPv4 representation address. Example: ::FFFF:192.168.1.120
    std::string ToString() const;

    // Returns true if the address is a wild card (all zero) address.
    bool IsWildcard() const;

    // Returns true if the address is a broadcast address.
    // Only IPv4 addresses can be broadcast addresses. In a broadcast address, all bits are one.
    // For a IPv6 address, returns always false.
    bool IsBroadcast() const;

    // Returns true if the address is a loopback address.
    // For IPv4, the loopback address is 127.0.0.1.
    // For IPv6, the loopback address is ::1.
    bool IsLoopback() const;

    // Returns true if the address is a multicast address.
    // IPv4 multicast addresses are in the 224.0.0.0 to 239.255.255.255 range
    // (the first four bits have the value 1110).
    // IPv6 multicast addresses are in the FFxx:x:x:x:x:x:x:x range.
    bool IsMulticast() const;

    // Returns true if the address is a unicast address.
    // An address is unicast if it is neither a wildcard, broadcast or multicast address.
    bool IsUnicast() const;

    // Returns true if the address is a link local unicast address.
    // IPv4 link local addresses are in the 169.254.0.0/16 range, according to RFC 3927.
    // IPv6 link local addresses have 1111 1110 10 as the first 10 bits, followed by 54 zeros.
    bool IsLinkLocal() const;

    // Returns true if the address is a site local unicast address.
    // IPv4 site local addresses are in on of the 10.0.0.0/24,
    // 192.168.0.0/16 or 172.16.0.0 to 172.31.255.255 ranges.
    // IPv6 site local addresses have 1111 1110 11 as the first
    // 10 bits, followed by 38 zeros.
    bool IsSiteLocal() const;

    // Returns true if the address is IPv4 compatible.
    // For IPv4 addresses, this is always true.
    // For IPv6, the address must be in the ::x:x range (the first 96 bits are zero).
    bool IsIPv4Compatible() const;

    // Returns true iff the address is an IPv4 mapped IPv6 address.
    // For IPv4 addresses, this is always true.
    // For IPv6, the address must be in the ::FFFF:x:x range.
    bool IsIPv4Mapped() const;

    // Returns true if the address is a well-known multicast address.
    // For IPv4, well-known multicast addresses are in the 224.0.0.0/8 range.
    // For IPv6, well-known multicast addresses are in the FF0x:x:x:x:x:x:x:x range.
    bool IsWellKnownMC() const;

    // Returns true if the address is a node-local multicast address.
    // IPv4 does not support node-local addresses, thus the result is
    // always false for an IPv4 address.
    // For IPv6, node-local multicast addresses are in the FFx1:x:x:x:x:x:x:x range.
    bool IsNodeLocalMC() const;

    // Returns true if the address is a link-local multicast address.
    // For IPv4, link-local multicast addresses are in the 224.0.0.0/24 range.
    // Note that this overlaps with the range for well-known multicast addresses.
    // For IPv6, link-local multicast addresses are in the FFx2:x:x:x:x:x:x:x range.
    bool IsLinkLocalMC() const;

    // Returns true if the address is a site-local multicast address.
    // For IPv4, site local multicast addresses are in the 239.255.0.0/16 range.
    // For IPv6, site-local multicast addresses are in the FFx5:x:x:x:x:x:x:x range.
    bool IsSiteLocalMC() const;

    // Returns true iff the address is a organization-local multicast address.
    // For IPv4, organization-local multicast addresses are in the 239.192.0.0/16 range.
    // For IPv6, organization-local multicast addresses are in the
    // FFx8:x:x:x:x:x:x:x range.
    bool IsOrgLocalMC() const;

    // Returns true iff the address is a global multicast address.
    // For IPv4, global multicast addresses are in the 224.0.1.0 to 238.255.255.255 range.
    // For IPv6, global multicast addresses are in the FFxF:x:x:x:x:x:x:x range.
    bool IsGlobalMC() const;

    bool operator ==(const IPAddress& addr) const;
    bool operator !=(const IPAddress& addr) const;
    bool operator <(const IPAddress& addr) const;
    bool operator <=(const IPAddress& addr) const;
    bool operator >(const IPAddress& addr) const;
    bool operator >=(const IPAddress& addr) const;

    // Returns the Length in bytes of the internal socket address structure.	
    SOCKET_LENGTH_t GetLength() const;

    // Returns the internal address structure.
    const void* GetAddr() const;

    // Returns the address family (AF_INET or AF_INET6) of the address.
    int GetAddressFamily() const;

    // Masks the IP address using the given netmask, which is usually a IPv4 subnet Mask.
    // Only supported for IPv4 addresses. The new address is (address & Mask).
    bool Mask(const IPAddress& Mask);

    // Masks the IP address using the given netmask, which is usually a IPv4 subnet Mask.
    // Only supported for IPv4 addresses. The new address is (address & Mask) | (set & ~Mask).
    bool Mask(const IPAddress& Mask, const IPAddress& set);

    // Tries to interpret the given address string as an
    // IP address in presentation format (dotted decimal for IPv4, hex string for IPv6).
    // Returns true and stores the IPAddress in result if the string contains a valid address.
    // Returns false and leaves result unchanged otherwise.
    static bool Parse(const std::string& addr, IPAddress& result);

    // Maximum Length in bytes of a socket address.
    enum
    {
        MAX_ADDRESS_LENGTH = sizeof(in6_addr) //sizeof(in_addr)
    };

protected:
    void Init(IPAddressImpl* pImpl);

private:
    IPAddressImpl* mImpl;
};

#endif // IPAddress_INCLUDED
