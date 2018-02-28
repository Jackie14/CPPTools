//////////////////////////////////////////////////////////////////////////
// SocketAddress.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "SocketAddress.h"
#include "IPAddress.h"
#include "DNS.h"
#include "RefCountedObject.h"
#include "NumberParser.h"
#include "NumberFormatter.h"
#include <algorithm>
#include <cstring>
#include <assert.h>

// SocketAddressImpl
class SocketAddressImpl: public RefCountedObject
{
public:
    virtual bool GetHost(IPAddress& ipAddr) const = 0;
    virtual UInt16 GetPort() const = 0;
    virtual SOCKET_LENGTH_t GetLength() const = 0;
    virtual const sockaddr* GetAddr() const = 0;
    virtual int GetAF() const = 0;

protected:
    SocketAddressImpl()
    {
    }

    virtual ~SocketAddressImpl()
    {
    }

private:
    SocketAddressImpl(const SocketAddressImpl&);
    SocketAddressImpl& operator =(const SocketAddressImpl&);
};

class IPv4SocketAddressImpl: public SocketAddressImpl
{
public:
    IPv4SocketAddressImpl()
    {
        std::memset(&mAddr, 0, sizeof(mAddr));
        mAddr.sin_family = AF_INET;
        socket_set_sin_len(&mAddr);
    }

    IPv4SocketAddressImpl(const sockaddr_in* addr)
    {
        std::memcpy(&mAddr, addr, sizeof(mAddr));
    }

    IPv4SocketAddressImpl(const void* addr, UInt16 port)
    {
        std::memset(&mAddr, 0, sizeof(mAddr));
        mAddr.sin_family = AF_INET;
        std::memcpy(&mAddr.sin_addr, addr, sizeof(mAddr.sin_addr));
        mAddr.sin_port = port;
    }

    bool GetHost(IPAddress& ipAddr) const
    {
        bool hasError = false;
        ipAddr = IPAddress(&mAddr.sin_addr, sizeof(mAddr.sin_addr), hasError);
        return !hasError;
    }

    UInt16 GetPort() const
    {
        return mAddr.sin_port;
    }

    SOCKET_LENGTH_t GetLength() const
    {
        return sizeof(mAddr);
    }

    const sockaddr* GetAddr() const
    {
        return reinterpret_cast<const sockaddr*>(&mAddr);
    }

    int GetAF() const
    {
        return mAddr.sin_family;
    }

private:
    sockaddr_in mAddr;
};

class IPv6SocketAddressImpl: public SocketAddressImpl
{
public:
    IPv6SocketAddressImpl(const sockaddr_in6* addr)
    {
        std::memcpy(&mAddr, addr, sizeof(mAddr));
    }

    IPv6SocketAddressImpl(const void* addr, UInt16 port)
    {
        std::memset(&mAddr, 0, sizeof(mAddr));
        mAddr.sin6_family = AF_INET6;
        socket_set_sin6_len(&mAddr);
        std::memcpy(&mAddr.sin6_addr, addr, sizeof(mAddr.sin6_addr));
        mAddr.sin6_port = port;
    }

    bool GetHost(IPAddress& ipAddr) const
    {
        bool hasError = false;
        ipAddr = IPAddress(&mAddr.sin6_addr, sizeof(mAddr.sin6_addr), hasError);
        return !hasError;
    }

    UInt16 GetPort() const
    {
        return mAddr.sin6_port;
    }

    SOCKET_LENGTH_t GetLength() const
    {
        return sizeof(mAddr);
    }

    const sockaddr* GetAddr() const
    {
        return reinterpret_cast<const sockaddr*>(&mAddr);
    }

    int GetAF() const
    {
        return mAddr.sin6_family;
    }

private:
    sockaddr_in6 mAddr;
};

// SocketAddress
SocketAddress::SocketAddress() : mImpl(NULL)
{
    mImpl = new IPv4SocketAddressImpl;
}

SocketAddress::SocketAddress(const IPAddress& addr, UInt16 port, bool& hasError) : mImpl(NULL)
{
    bool ret = Init(addr, port);
    hasError = !ret;
}

SocketAddress::SocketAddress(const std::string& addr, UInt16 port, bool& hasError) : mImpl(NULL)
{
    bool ret = Init(addr, port);
    hasError = !ret;
}

SocketAddress::SocketAddress(const std::string& addr, const std::string& port,
        bool& hasError) : mImpl(NULL)
{
    bool ret = Init(addr, ResolveService(port));
    hasError = !ret;
}

SocketAddress::SocketAddress(const std::string& hostAndPort, bool& hasError) : mImpl(NULL)
{
    hasError = false;
    std::string host;
    std::string port;
    std::string::const_iterator it = hostAndPort.begin();
    std::string::const_iterator end = hostAndPort.end();
    if (*it == '[')
    {
        ++it;
        while (it != end && *it != ']')
        {
            host += *it++;
        }
        if (it == end)
        {
            hasError = true;
            assert("Malformed IPv6 address");
            return;
        }
        ++it;
    }
    else
    {
        while (it != end && *it != ':')
        {
            host += *it++;
        }
    }
    if (it != end && *it == ':')
    {
        ++it;
        while (it != end)
        {
            port += *it++;
        }
    }
    else
    {
        hasError = true;
        assert("Missing port number");
        return;
    }

    bool ret = Init(host, ResolveService(port));
    hasError = !ret;
}

SocketAddress::SocketAddress(const SocketAddress& addr) : mImpl(NULL)
{
    mImpl = addr.mImpl;
    if(mImpl)
    {
        mImpl->Duplicate();
    }
}

SocketAddress::SocketAddress(const sockaddr* addr,
        SOCKET_LENGTH_t length, bool& hasError) : mImpl(NULL)
{
    hasError = false;
    if (length == sizeof(sockaddr_in))
    {
        mImpl = new IPv4SocketAddressImpl(reinterpret_cast<const sockaddr_in*>(addr));
    }
    else if (length == sizeof(sockaddr_in6))
    {
        mImpl = new IPv6SocketAddressImpl(reinterpret_cast<const sockaddr_in6*>(addr));
    }
    else
    {
        hasError = true;
        assert("Invalid address length");
    }
}

SocketAddress::~SocketAddress()
{
    if(mImpl)
    {
        mImpl->Release();
    }
}

SocketAddress& SocketAddress::operator =(const SocketAddress& addr)
{
    if (&addr != this && mImpl)
    {
        mImpl->Release();
        mImpl = addr.mImpl;
        mImpl->Duplicate();
    }
    return *this;
}

bool SocketAddress::GetHost(IPAddress& ipAddr) const
{
    if(!mImpl)
    {
        return false;
    }

    return mImpl->GetHost(ipAddr);
}

UInt16 SocketAddress::GetPort() const
{
    if(!mImpl)
    {
        return 0;
    }

    return ntohs(mImpl->GetPort());
}

SOCKET_LENGTH_t SocketAddress::GetLength() const
{
    if(!mImpl)
    {
        return 0;
    }

    return mImpl->GetLength();
}

const sockaddr* SocketAddress::GetAddr() const
{
    if(!mImpl)
    {
        return NULL;
    }

    return mImpl->GetAddr();
}

int SocketAddress::GetAF() const
{
    if(!mImpl)
    {
        return -1;
    }

    return mImpl->GetAF();
}

std::string SocketAddress::ToString() const
{
    std::string result;
    bool ret = true;
    IPAddress ipAddr;
    ret = GetHost(ipAddr);
    if(!ret)
    {
        return "";
    }

    if (ipAddr.GetFamily() == IPAddress::IPv6)
    {
        result.append("[");
    }
    result.append(ipAddr.ToString());

    if (ipAddr.GetFamily() == IPAddress::IPv6)
    {
        result.append("]");
    }
    result.append(":");
    NumberFormatter::Append(result, GetPort());
    return result;
}

bool SocketAddress::Init(const IPAddress& host, UInt16 port)
{
    bool ret = true;
    if (host.GetFamily() == IPAddress::IPv4)
    {
        mImpl = new IPv4SocketAddressImpl(host.GetAddr(), htons(port));
    }
    else if (host.GetFamily() == IPAddress::IPv6)
    {
        mImpl = new IPv6SocketAddressImpl(host.GetAddr(), htons(port));
    }
    else
    {
        ret = false;
        assert("Unsupported IP address family");
    }

    return ret;
}

bool SocketAddress::Init(const std::string& host, UInt16 port)
{
    bool ret = true;
    IPAddress ip;
    if (IPAddress::Parse(host, ip))
    {
        ret = Init(ip, port);
        if(!ret)
        {
            return ret;
        }

        std::string::size_type pos = host.rfind('%');
        if (std::string::npos != pos)
        {
            std::string scope(host, pos + 1);
            if (']' == scope[scope.length() - 1])
            {
                scope.resize(scope.length() - 1);
            }
            ((sockaddr_in6*)GetAddr())->sin6_scope_id =
                    if_nametoindex(scope.c_str());
        }
    }
    else
    {
        HostEntry he;
        ret = DNS::HostByName(host, he);
        if (ret && he.GetAddresses().size() > 0)
        {
            ret = Init(he.GetAddresses()[0], port);
        }
        else
        {
            ret = false;
            assert("No address found for the host");
        }
    }

    return ret;
}

UInt16 SocketAddress::ResolveService(const std::string& service)
{
    unsigned port;
    if (NumberParser::ParseUnsignedInt(service, port) && port <= 0xFFFF)
    {
        return (UInt16)port;
    }
    else
    {
        servent* se = getservbyname(service.c_str(), NULL);
        if (se)
        {
            return ntohs(se->s_port);
        }
        else
        {
            assert("Service not found");
            return 0;
        }
    }
}
