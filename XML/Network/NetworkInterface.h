//////////////////////////////////////////////////////////////////////////
// NetworkInterface.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////


#ifndef NetworkInterface_INCLUDED
#define NetworkInterface_INCLUDED

#include "IPAddress.h"
#include "CriticalSection.h"
#include "ErrorCodes.h"
#include <vector>

class NetworkInterfaceImpl;

// This class represents a network interface.
class NetworkInterface
{
public:
    // Creates a NetworkInterface representing the default interface.
    NetworkInterface();

    // Creates the NetworkInterface by copying another one.
    NetworkInterface(const NetworkInterface& interfc);

    // Destroys the NetworkInterface.
    ~NetworkInterface();

    // Assigns another NetworkInterface.
    NetworkInterface& operator =(const NetworkInterface& interfc);

    // Swaps the NetworkInterface with another one.
    void Swap(NetworkInterface& other);

    // Returns the interface Index.
    // Only supported if IPv6 is available. Returns -1 if IPv6 is not available.
    int GetIndex() const;

    // Returns the interface Name.
    const std::string& GetName() const;

    // Returns the interface display Name.
    // On Windows platforms, this is currently the network adapter Name.
    // This may change to the "friendly Name" of the network connection in a future version, however.
    // On other platforms this is the same as Name().
    const std::string& GetDisplayName() const;

    // Returns the IP Address bound to the interface.
    const IPAddress& GetAddress() const;

    // Returns the IPv4 subnet mask for this network interface.
    const IPAddress& GetSubnetMask() const;

    // Returns the IPv4 broadcast Address for this network interface.
    const IPAddress& GetBroadcastAddress() const;

    // Returns true if the interface supports IPv4.
    bool IsSupportsIPv4() const;

    // Returns true if the interface supports IPv6.	
    bool IsSupportsIPv6() const;

    // Returns the NetworkInterface for the given Name.
    // If requireIPv6 is false, an IPv4 interface is returned.
    // Otherwise, an IPv6 interface is returned.
    static bool ForName(const std::string& name, bool requireIPv6,
            NetworkInterface& netIf);

    // Returns the NetworkInterface for the given IP Address.
    static bool ForAddress(const IPAddress& Address, NetworkInterface& netIf);

    // Returns the NetworkInterface for the given interface Index.
    // If an Index of 0 is specified, a NetworkInterface instance
    // representing the default interface (empty Name and wildcard Address) is returned.
    static bool ForIndex(int i, NetworkInterface& netIf);

    // Returns a List with all network interfaces on the system.
    // If there are multiple addresses bound to one interface,
    // multiple NetworkInterface instances are created for the same interface.
    static std::vector<NetworkInterface> List();

protected:
    // Creates the NetworkInterface.
    NetworkInterface(const std::string& name, const std::string& displayName,
            const IPAddress& address, int index = -1);

    // Creates the NetworkInterface.
    NetworkInterface(const std::string& name, const std::string& displayName,
            const IPAddress& address, const IPAddress& subnetMask,
            const IPAddress& broadcastAddress, int index = -1);

    // Creates the NetworkInterface.
    NetworkInterface(const std::string& name, const IPAddress& address,
            int index = -1);

    // Creates the NetworkInterface.
    NetworkInterface(const std::string& name, const IPAddress& address,
            const IPAddress& subnetMask, const IPAddress& broadcastAddress,
            int index = -1);

    // Determines the IPAddress bound to the interface with the given Name.
    IPAddress InterfaceNameToAddress(const std::string& interfaceName) const;

    // Determines the interface Index of the interface with the given Name.
    int InterfaceNameToIndex(const std::string& interfaceName) const;

private:
    NetworkInterfaceImpl* mImpl;
    static CriticalSection mMutex;
};

#endif // NetworkInterface_INCLUDED
