//////////////////////////////////////////////////////////////////////////
// HostEntry.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef HostEntry_INCLUDED
#define HostEntry_INCLUDED

#include "SocketDefs.h"
#include "IPAddress.h"
#include <vector>

// This class stores information about a host
// such as host name, alias names and a list of IP addresses.
class HostEntry
{
public:
    // Creates an empty HostEntry.
    HostEntry();

    // Creates the HostEntry from the data in a hostent structure.
    HostEntry(hostent* entry);

    // Creates the HostEntry from the data in an addrinfo structure.
    HostEntry(addrinfo* info);

    // Creates the HostEntry by copying another one.
    HostEntry(const HostEntry& entry);

    // Assigns another HostEntry.
    HostEntry& operator =(const HostEntry& entry);

    // Destroys the HostEntry.
    ~HostEntry();

    // Returns the canonical host name.
    const std::string& GetName() const
    {
        return mName;
    }

    // Returns a vector containing alias names for the host name.
    const std::vector<std::string>& GetAliases() const
    {
        return mAliases;
    }

    // Returns a vector containing the IPAddresse for the host.
    const std::vector<IPAddress>& GetAddresses() const
    {
        return mAddresses;
    }

private:
    std::string mName;
    std::vector<std::string> mAliases;
    std::vector<IPAddress> mAddresses;
};

#endif // HostEntry_INCLUDED
