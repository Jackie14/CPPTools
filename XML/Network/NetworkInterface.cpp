//////////////////////////////////////////////////////////////////////////
// NetworkInterface.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "NetworkInterface.h"
#include "Socket.h"
#include "Log.h"
#include "NumberFormatter.h"
#include "RefCountedObject.h"
#include <cstring>

//////////////////////////////////////////////////////////////////////////
// NetworkInterfaceImpl
class NetworkInterfaceImpl: public RefCountedObject
{
public:
    NetworkInterfaceImpl();
    NetworkInterfaceImpl(const std::string& name,
            const std::string& displayName, const IPAddress& address,
            int index = -1);
    NetworkInterfaceImpl(const std::string& name,
            const std::string& displayName, const IPAddress& address,
            const IPAddress& subnetMask, const IPAddress& broadcastAddress,
            int index = -1);

    int GetIndex() const;
    const std::string& GetName() const;
    const std::string& GetDisplayName() const;
    const IPAddress& GetAddress() const;
    const IPAddress& GetSubnetMask() const;
    const IPAddress& GetBroadcastAddress() const;

protected:
    ~NetworkInterfaceImpl();

private:
    std::string mName;
    std::string mDisplayName;
    IPAddress mAddress;
    IPAddress mSubnetMask;
    IPAddress mBroadcastAddress;
    int mIndex;
};

NetworkInterfaceImpl::NetworkInterfaceImpl() :
    mIndex(-1)
{
}

NetworkInterfaceImpl::NetworkInterfaceImpl(const std::string& name,
        const std::string& displayName, const IPAddress& address, int index) :
    mName(name), mDisplayName(displayName), mAddress(address), mIndex(index)
{
    if (mIndex == -1) // IPv4
    {
        bool hasError = false;
        ifreq ifr;
        std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);
        Socket ds(IPAddress::IPv4, SOCK_DGRAM);
        ds.ioctl(SIOCGIFNETMASK, &ifr);
        if (ifr.ifr_addr.sa_family == AF_INET)
        {
            mSubnetMask = IPAddress(&reinterpret_cast<const sockaddr_in*>(&ifr.ifr_addr)->sin_addr,
                            sizeof(in_addr), hasError);
        }
        if (!mAddress.IsLoopback())
        {
            try
            {
                // Not every interface (e.g. loopback) has a broadcast Address
                ds.ioctl(SIOCGIFBRDADDR, &ifr);
                if (ifr.ifr_addr.sa_family == AF_INET)
                {
                    mBroadcastAddress = IPAddress(&reinterpret_cast<const sockaddr_in*> (&ifr.ifr_addr)->sin_addr,
                            sizeof(in_addr), hasError);
                }
            }
            catch (...)
            {
            }
        }
    }
}

NetworkInterfaceImpl::NetworkInterfaceImpl(const std::string& name,
        const std::string& displayName, const IPAddress& address,
        const IPAddress& subnetMask, const IPAddress& broadcastAddress,
        int index) :
    mName(name), mDisplayName(displayName), mAddress(address), mSubnetMask(
            subnetMask), mBroadcastAddress(broadcastAddress), mIndex(index)
{
}

NetworkInterfaceImpl::~NetworkInterfaceImpl()
{
}

inline int NetworkInterfaceImpl::GetIndex() const
{
    return mIndex;
}

inline const std::string& NetworkInterfaceImpl::GetName() const
{
    return mName;
}

inline const std::string& NetworkInterfaceImpl::GetDisplayName() const
{
    return mDisplayName;
}

inline const IPAddress& NetworkInterfaceImpl::GetAddress() const
{
    return mAddress;
}

inline const IPAddress& NetworkInterfaceImpl::GetSubnetMask() const
{
    return mSubnetMask;
}

inline const IPAddress& NetworkInterfaceImpl::GetBroadcastAddress() const
{
    return mBroadcastAddress;
}

//////////////////////////////////////////////////////////////////////////
// NetworkInterface
CriticalSection NetworkInterface::mMutex;
NetworkInterface::NetworkInterface() :
    mImpl(new NetworkInterfaceImpl)
{
}

NetworkInterface::NetworkInterface(const NetworkInterface& interfc) :
    mImpl(interfc.mImpl)
{
    mImpl->Duplicate();
}

NetworkInterface::NetworkInterface(const std::string& name,
        const std::string& displayName, const IPAddress& address, int index) :
    mImpl(new NetworkInterfaceImpl(name, displayName, address, index))
{
}

NetworkInterface::NetworkInterface(const std::string& name,
        const std::string& displayName, const IPAddress& address,
        const IPAddress& subnetMask, const IPAddress& broadcastAddress,
        int index) :
    mImpl(new NetworkInterfaceImpl(name, displayName, address, subnetMask,
            broadcastAddress, index))
{
}

NetworkInterface::NetworkInterface(const std::string& name,
        const IPAddress& address, int index) :
    mImpl(new NetworkInterfaceImpl(name, name, address, index))
{
}

NetworkInterface::NetworkInterface(const std::string& name,
        const IPAddress& address, const IPAddress& subnetMask,
        const IPAddress& broadcastAddress, int index) :
    mImpl(new NetworkInterfaceImpl(name, name, address, subnetMask,
            broadcastAddress, index))
{
}

NetworkInterface::~NetworkInterface()
{
    mImpl->Release();
}

NetworkInterface& NetworkInterface::operator =(const NetworkInterface& interfc)
{
    NetworkInterface tmp(interfc);
    Swap(tmp);
    return *this;
}

void NetworkInterface::Swap(NetworkInterface& other)
{
    std::swap(mImpl, other.mImpl);
}

int NetworkInterface::GetIndex() const
{
    return mImpl->GetIndex();
}

const std::string& NetworkInterface::GetName() const
{
    return mImpl->GetName();
}

const std::string& NetworkInterface::GetDisplayName() const
{
    return mImpl->GetDisplayName();
}

const IPAddress& NetworkInterface::GetAddress() const
{
    return mImpl->GetAddress();
}

const IPAddress& NetworkInterface::GetSubnetMask() const
{
    return mImpl->GetSubnetMask();
}

const IPAddress& NetworkInterface::GetBroadcastAddress() const
{
    return mImpl->GetBroadcastAddress();
}

bool NetworkInterface::IsSupportsIPv4() const
{
    return mImpl->GetIndex() == -1;
}

bool NetworkInterface::IsSupportsIPv6() const
{
    return mImpl->GetIndex() != -1;
}

bool NetworkInterface::ForName(const std::string& name,
        bool requireIPv6, NetworkInterface& netIf)
{
    std::vector<NetworkInterface> ifs = List();
    for (std::vector<NetworkInterface>::const_iterator it = ifs.begin(); it
            != ifs.end(); ++it)
    {
        if (it->GetName() == name && ((requireIPv6 && it->IsSupportsIPv6())
                || !requireIPv6))
        {
            netIf = (*it);
            return true;
        }
    }

    return false;
}

bool NetworkInterface::ForAddress(const IPAddress& addr, NetworkInterface& netIf)
{
    std::vector<NetworkInterface> ifs = List();
    for (std::vector<NetworkInterface>::const_iterator it = ifs.begin(); it
            != ifs.end(); ++it)
    {
        if (it->GetAddress() == addr)
        {
            netIf = (*it);
            return true;
        }
    }

    return false;
}

bool NetworkInterface::ForIndex(int i, NetworkInterface& netIf)
{
    if(i < 0)
    {
        return false;
    }

    std::vector<NetworkInterface> ifs = List();
    for (std::vector<NetworkInterface>::const_iterator it = ifs.begin(); it
            != ifs.end(); ++it)
    {
        if (it->GetIndex() == i)
        {
            netIf = (*it);
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
std::vector<NetworkInterface> NetworkInterface::List()
{
    AutoCriticalSection lock(&mMutex);
    std::vector<NetworkInterface> result;
    Socket sock(SOCK_DGRAM);
    sock.Init(AF_INET);
    int lastlen = 0;
    int len = 100 * sizeof(ifreq);
    char* buf = 0;
    try
    {
        ifconf ifc;
        for (;;)
        {
            buf = new char[len];
            ifc.ifc_len = len;
            ifc.ifc_buf = buf;
            if (::ioctl(sock.Sockfd(), SIOCGIFCONF, &ifc) < 0)
            {
                if (errno != EINVAL || lastlen != 0)
                {
                    LOG(LogError, "Cannot get network adapter List");
                    delete []buf;
                    return result;
                }
            }
            else
            {
                if (ifc.ifc_len == lastlen)
                {
                    break;
                }
                lastlen = ifc.ifc_len;
            }
            len += 10 * sizeof(struct ifreq);
            delete[] buf;
        }

        for (const char* ptr = buf; ptr < buf + ifc.ifc_len;)
        {
            const ifreq* ifr = reinterpret_cast<const ifreq*> (ptr);
            IPAddress addr;
            bool haveAddr = false;
            bool hasError = false;
            switch (ifr->ifr_addr.sa_family)
            {
            case AF_INET6:
                addr = IPAddress(&reinterpret_cast<const sockaddr_in6*>(&ifr->ifr_addr)->sin6_addr,
                                sizeof(in6_addr), hasError);
                haveAddr = true;
                break;
            case AF_INET:
                addr = IPAddress(&reinterpret_cast<const sockaddr_in*>(&ifr->ifr_addr)->sin_addr,
                                sizeof(in_addr), hasError);
                haveAddr = true;
                break;
            default:
                break;
            }
            if (haveAddr)
            {
                int index = if_nametoindex(ifr->ifr_name);
                std::string name(ifr->ifr_name);
                result.push_back(NetworkInterface(name, name, addr, index));
            }
            ptr += sizeof(ifreq);
        }
    }
    catch (...)
    {
        delete[] buf;
        //throw;
    }
    delete[] buf;
    return result;
}
