//////////////////////////////////////////////////////////////////////////
// DNS.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef DNS_INCLUDED
#define DNS_INCLUDED

#include "SocketDefs.h"
#include "IPAddress.h"
#include "HostEntry.h"

// This class provides an interface to the domain name service.
class DNS
{
public:
    // Returns a HostEntry containing the DNS information for the host with the given name.
    static bool HostByName(const std::string& hostname, HostEntry& hostEntry);

    // Returns a HostEntry containing the DNS information for the host with the given IP address.
    static bool HostByAddress(const IPAddress& address, HostEntry& hostEntry);

    // Returns a HostEntry containing the DNS information
    // for the host with the given IP address or host name.
    static bool Resolve(const std::string& address, HostEntry& hostEntry);

    // Convenience method that calls resolve(address) and returns the first address from the HostInfo.
    static bool ResolveOne(const std::string& address, IPAddress& ipAddr);

    // Returns a HostEntry containing the DNS information for this host.
    static bool ThisHost(HostEntry& hostEntry);

    // Returns the host name of this host.
    static std::string HostName();
};

#endif // DNS_INCLUDED
