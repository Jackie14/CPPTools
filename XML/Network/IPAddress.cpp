//////////////////////////////////////////////////////////////////////////
// IPAddress.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "IPAddress.h"
#include "NumberFormatter.h"
#include "Types.h"
#include "RefCountedObject.h"
#include <algorithm>
#include <cstring>
#include <assert.h>

// IPAddressImpl
class IPAddressImpl: public RefCountedObject
{
public:
    virtual std::string ToString() const = 0;
    virtual SOCKET_LENGTH_t GetLength() const = 0;
    virtual const void* GetAddr() const = 0;
    virtual IPAddress::IPFamily GetFamily() const = 0;
    virtual int GetAddressFamily() const = 0;
    virtual bool IsWildcard() const = 0;
    virtual bool IsBroadcast() const = 0;
    virtual bool IsLoopback() const = 0;
    virtual bool IsMulticast() const = 0;
    virtual bool IsLinkLocal() const = 0;
    virtual bool IsSiteLocal() const = 0;
    virtual bool IsIPv4Mapped() const = 0;
    virtual bool IsIPv4Compatible() const = 0;
    virtual bool IsWellKnownMC() const = 0;
    virtual bool IsNodeLocalMC() const = 0;
    virtual bool IsLinkLocalMC() const = 0;
    virtual bool IsSiteLocalMC() const = 0;
    virtual bool IsOrgLocalMC() const = 0;
    virtual bool IsGlobalMC() const = 0;
    virtual bool Mask(const IPAddressImpl* pMask, const IPAddressImpl* pSet) = 0;
    virtual IPAddressImpl* Clone() const = 0;

protected:
    IPAddressImpl()
    {
    }

    virtual ~IPAddressImpl()
    {
    }

private:
    IPAddressImpl(const IPAddressImpl&);
    IPAddressImpl& operator =(const IPAddressImpl&);
};

class IPv4AddressImpl: public IPAddressImpl
{
public:
    IPv4AddressImpl()
    {
        std::memset(&mAddr, 0, sizeof(mAddr));
    }

    IPv4AddressImpl(const void* addr)
    {
        std::memcpy(&mAddr, addr, sizeof(mAddr));
    }

    std::string ToString() const
    {
        const UInt8* bytes = reinterpret_cast<const UInt8*> (&mAddr);
        std::string result;
        result.reserve(16);
        NumberFormatter::Append(result, bytes[0]);
        result.append(".");
        NumberFormatter::Append(result, bytes[1]);
        result.append(".");
        NumberFormatter::Append(result, bytes[2]);
        result.append(".");
        NumberFormatter::Append(result, bytes[3]);
        return result;
    }

    SOCKET_LENGTH_t GetLength() const
    {
        return sizeof(mAddr);
    }

    const void* GetAddr() const
    {
        return &mAddr;
    }

    IPAddress::IPFamily GetFamily() const
    {
        return IPAddress::IPv4;
    }

    int GetAddressFamily() const
    {
        return AF_INET;
    }

    bool IsWildcard() const
    {
        return mAddr.s_addr == INADDR_ANY;
    }

    bool IsBroadcast() const
    {
        return mAddr.s_addr == INADDR_NONE;
    }

    bool IsLoopback() const
    {
        // 127.0.0.1
        return ntohl(mAddr.s_addr) == 0x7F000001;
    }

    bool IsMulticast() const
    {
        // 224.0.0.0/24 to 239.0.0.0/24
        return (ntohl(mAddr.s_addr) & 0xF0000000) == 0xE0000000;
    }

    bool IsLinkLocal() const
    {
        // 169.254.0.0/16
        return (ntohl(mAddr.s_addr) & 0xFFFF0000) == 0xA9FE0000;
    }

    bool IsSiteLocal() const
    {
        UInt32 addr = ntohl(mAddr.s_addr);
        return (addr & 0xFF000000) == 0x0A000000 || // 10.0.0.0/24
                (addr & 0xFFFF0000) == 0xC0A80000 || // 192.68.0.0/16
                (addr >= 0xAC100000 && addr <= 0xAC1FFFFF); // 172.16.0.0 to 172.31.255.255
    }

    bool IsIPv4Compatible() const
    {
        return true;
    }

    bool IsIPv4Mapped() const
    {
        return true;
    }

    bool IsWellKnownMC() const
    {
        // 224.0.0.0/8
        return (ntohl(mAddr.s_addr) & 0xFFFFFF00) == 0xE0000000;
    }

    bool IsNodeLocalMC() const
    {
        return false;
    }

    bool IsLinkLocalMC() const
    {
        // 244.0.0.0/24
        return (ntohl(mAddr.s_addr) & 0xFF000000) == 0xE0000000;
    }

    bool IsSiteLocalMC() const
    {
        // 239.255.0.0/16
        return (ntohl(mAddr.s_addr) & 0xFFFF0000) == 0xEFFF0000;
    }

    bool IsOrgLocalMC() const
    {
        // 239.192.0.0/16
        return (ntohl(mAddr.s_addr) & 0xFFFF0000) == 0xEFC00000;
    }

    bool IsGlobalMC() const
    {
        // 224.0.1.0 to 238.255.255.255
        UInt32 addr = ntohl(mAddr.s_addr);
        return addr >= 0xE0000100 && addr <= 0xEE000000;
    }

    static IPv4AddressImpl* Parse(const std::string& addr)
    {
        if (addr.empty())
        {
            return NULL;
        }

        in_addr ia;
        if (inet_aton(addr.c_str(), &ia))
        {
            return new IPv4AddressImpl(&ia);
        }
        else
        {
            return NULL;
        }
    }

    bool Mask(const IPAddressImpl* pMask, const IPAddressImpl* pSet)
    {
        mAddr.s_addr &= static_cast<const IPv4AddressImpl*>(pMask)->mAddr.s_addr;
        mAddr.s_addr |= static_cast<const IPv4AddressImpl*>(pSet)->mAddr.s_addr
                        & ~static_cast<const IPv4AddressImpl*>(pMask)->mAddr.s_addr;
        return true;
    }

    IPAddressImpl* Clone() const
    {
        return new IPv4AddressImpl(&mAddr);
    }

private:
    in_addr mAddr;
};

class IPv6AddressImpl: public IPAddressImpl
{
public:
    IPv6AddressImpl()
    {
        mScope = 0;
        std::memset(&mAddr, 0, sizeof(mAddr));
    }

    IPv6AddressImpl(const void* addr)
    {
        mScope = 0;
        std::memcpy(&mAddr, addr, sizeof(mAddr));
    }

    IPv6AddressImpl(const void* addr, unsigned int scope)
    {
        mScope = scope;
        std::memcpy(&mAddr, addr, sizeof(mAddr));
    }

    std::string ToString() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        if (IsIPv4Compatible() || IsIPv4Mapped())
        {
            std::string result;
            result.reserve(24);
            if (words[5] == 0)
            {
                result.append("::");
            }
            else
            {
                result.append("::FFFF:");
            }

            const UInt8* bytes = reinterpret_cast<const UInt8*>(&mAddr);
            NumberFormatter::Append(result, bytes[12]);
            result.append(".");
            NumberFormatter::Append(result, bytes[13]);
            result.append(".");
            NumberFormatter::Append(result, bytes[14]);
            result.append(".");
            NumberFormatter::Append(result, bytes[15]);
            return result;
        }
        else
        {
            std::string result;
            result.reserve(46);
            bool zeroSequence = false;
            int i = 0;
            while (i < 8)
            {
                if (!zeroSequence && words[i] == 0)
                {
                    int zi = i;
                    while (zi < 8 && words[zi] == 0)
                    {
                        ++zi;
                    }

                    if (zi > i + 1)
                    {
                        i = zi;
                        result.append(":");
                        zeroSequence = true;
                    }
                }
                if (i > 0)
                {
                    result.append(":");
                }
                if (i < 8)
                {
                    NumberFormatter::AppendHex(result, ntohs(words[i++]));
                }
            }
            if (mScope > 0)
            {
                result.append("%");
                char buffer[IFNAMSIZ];
                if (if_indextoname(mScope, buffer))
                {
                    result.append(buffer);
                }
                else
                {
                    NumberFormatter::Append(result, mScope);
                }
            }
            return result;
        }
    }

    SOCKET_LENGTH_t GetLength() const
    {
        return sizeof(mAddr);
    }

    const void* GetAddr() const
    {
        return &mAddr;
    }

    IPAddress::IPFamily GetFamily() const
    {
        return IPAddress::IPv6;
    }

    int GetAddressFamily() const
    {
        return AF_INET6;
    }

    unsigned int GetScope() const
    {
        return mScope;
    }

    bool IsWildcard() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0
                && words[4] == 0 && words[5] == 0 && words[6] == 0 && words[7]
                == 0;
    }

    bool IsBroadcast() const
    {
        return false;
    }

    bool IsLoopback() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0
                && words[4] == 0 && words[5] == 0 && words[6] == 0 && words[7]
                == 1;
    }

    bool IsMulticast() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFE0) == 0xFF00;
    }

    bool IsLinkLocal() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFE0) == 0xFE80;
    }

    bool IsSiteLocal() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFE0) == 0xFEC0;
    }

    bool IsIPv4Compatible() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0
                && words[4] == 0 && words[5] == 0;
    }

    bool IsIPv4Mapped() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0
                && words[4] == 0 && words[5] == 0xFFFF;
    }

    bool IsWellKnownMC() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFF0) == 0xFF00;
    }

    bool IsNodeLocalMC() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFEF) == 0xFF01;
    }

    bool IsLinkLocalMC() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFEF) == 0xFF02;
    }

    bool IsSiteLocalMC() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFEF) == 0xFF05;
    }

    bool IsOrgLocalMC() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFEF) == 0xFF08;
    }

    bool IsGlobalMC() const
    {
        const UInt16* words = reinterpret_cast<const UInt16*>(&mAddr);
        return (words[0] & 0xFFEF) == 0xFF0F;
    }

    static IPv6AddressImpl* Parse(const std::string& addr)
    {
        if (addr.empty())
        {
            return NULL;
        }

        in6_addr ia;
        std::string::size_type pos = addr.find('%');
        if (std::string::npos != pos)
        {
            std::string::size_type start = ('[' == addr[0]) ? 1 : 0;
            std::string unscopedAddr(addr, start, pos - start);
            std::string scope(addr, pos + 1, addr.size() - start - pos);
            unsigned int scopeId(0);
            if (!(scopeId = if_nametoindex(scope.c_str())))
            {
                return NULL;
            }

            if (inet_pton(AF_INET6, unscopedAddr.c_str(), &ia) == 1)
            {
                return new IPv6AddressImpl(&ia, scopeId);
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            if (inet_pton(AF_INET6, addr.c_str(), &ia) == 1)
            {
                return new IPv6AddressImpl(&ia);
            }
            else
            {
                return NULL;
            }
        }
    }

    bool Mask(const IPAddressImpl* pMask, const IPAddressImpl* pSet)
    {
        return false;
    }

    IPAddressImpl* Clone() const
    {
        return new IPv6AddressImpl(&mAddr, mScope);
    }

private:
    in6_addr mAddr;
    unsigned int mScope;
};

// IPAddress
IPAddress::IPAddress() :
    mImpl(new IPv4AddressImpl)
{
}

IPAddress::IPAddress(const IPAddress& addr) :
    mImpl(addr.mImpl)
{
    mImpl->Duplicate();
}

IPAddress::IPAddress(IPFamily family) :
    mImpl(0)
{
    if (family == IPv4)
    {
        mImpl = new IPv4AddressImpl();
    }
    else if (family == IPv6)
    {
        mImpl = new IPv6AddressImpl();
    }
    else
    {
        assert("Invalid address family");
    }
}

IPAddress::IPAddress(const std::string& addr, bool& hasError)
{
    hasError = false;
    mImpl = IPv4AddressImpl::Parse(addr);
    if (!mImpl)
    {
        mImpl = IPv6AddressImpl::Parse(addr);
    }

    if (!mImpl)
    {
        hasError = true;
        assert("Invalid address");
    }
}

IPAddress::IPAddress(const std::string& addr, IPFamily family, bool& hasError) :
    mImpl(0)
{
    hasError = false;
    if (family == IPv4)
    {
        mImpl = IPv4AddressImpl::Parse(addr);
    }
    else if (family == IPv6)
    {
        mImpl = IPv6AddressImpl::Parse(addr);
    }
    else
    {
        hasError = true;
        assert("Invalid address family");
    }

    if (!mImpl)
    {
        hasError = true;
        assert("Invalid address");
    }
}

IPAddress::IPAddress(const void* addr, SOCKET_LENGTH_t Length, bool& hasError)
{
    hasError = false;
    if (Length == sizeof(struct in_addr))
    {
        mImpl = new IPv4AddressImpl(addr);
    }
    else if (Length == sizeof(struct in6_addr))
    {
        mImpl = new IPv6AddressImpl(addr);
    }
    else
    {
        hasError = true;
        assert("Invalid address Length");
    }
}

IPAddress::IPAddress(const void* addr, SOCKET_LENGTH_t length,
        unsigned int scope, bool& hasError)
{
    hasError = false;
    if (length == sizeof(in_addr))
    {
        mImpl = new IPv4AddressImpl(addr);
    }
    else if (length == sizeof(in6_addr))
    {
        mImpl = new IPv6AddressImpl(addr, scope);
    }
    else
    {
        hasError = true;
        assert("Invalid address Length");
    }
}

IPAddress::~IPAddress()
{
    mImpl->Release();
}

IPAddress& IPAddress::operator =(const IPAddress& addr)
{
    if (&addr != this)
    {
        mImpl->Release();
        mImpl = addr.mImpl;
        mImpl->Duplicate();
    }
    return *this;
}

IPAddress::IPFamily IPAddress::GetFamily() const
{
    return mImpl->GetFamily();
}

std::string IPAddress::ToString() const
{
    return mImpl->ToString();
}

bool IPAddress::IsWildcard() const
{
    return mImpl->IsWildcard();
}

bool IPAddress::IsBroadcast() const
{
    return mImpl->IsBroadcast();
}

bool IPAddress::IsLoopback() const
{
    return mImpl->IsLoopback();
}

bool IPAddress::IsMulticast() const
{
    return mImpl->IsMulticast();
}

bool IPAddress::IsUnicast() const
{
    return !IsWildcard() && !IsBroadcast() && !IsMulticast();
}

bool IPAddress::IsLinkLocal() const
{
    return mImpl->IsLinkLocal();
}

bool IPAddress::IsSiteLocal() const
{
    return mImpl->IsSiteLocal();
}

bool IPAddress::IsIPv4Compatible() const
{
    return mImpl->IsIPv4Compatible();
}

bool IPAddress::IsIPv4Mapped() const
{
    return mImpl->IsIPv4Mapped();
}

bool IPAddress::IsWellKnownMC() const
{
    return mImpl->IsWellKnownMC();
}

bool IPAddress::IsNodeLocalMC() const
{
    return mImpl->IsNodeLocalMC();
}

bool IPAddress::IsLinkLocalMC() const
{
    return mImpl->IsLinkLocalMC();
}

bool IPAddress::IsSiteLocalMC() const
{
    return mImpl->IsSiteLocalMC();
}

bool IPAddress::IsOrgLocalMC() const
{
    return mImpl->IsOrgLocalMC();
}

bool IPAddress::IsGlobalMC() const
{
    return mImpl->IsGlobalMC();
}

bool IPAddress::operator ==(const IPAddress& a) const
{
    SOCKET_LENGTH_t l1 = GetLength();
    SOCKET_LENGTH_t l2 = a.GetLength();
    if (l1 == l2)
    {
        return std::memcmp(GetAddr(), a.GetAddr(), l1) == 0;
    }
    else
    {
        return false;
    }
}

bool IPAddress::operator !=(const IPAddress& a) const
{
    SOCKET_LENGTH_t l1 = GetLength();
    SOCKET_LENGTH_t l2 = a.GetLength();
    if (l1 == l2)
    {
        return std::memcmp(GetAddr(), a.GetAddr(), l1) != 0;
    }
    else
    {
        return true;
    }
}

bool IPAddress::operator <(const IPAddress& a) const
{
    SOCKET_LENGTH_t l1 = GetLength();
    SOCKET_LENGTH_t l2 = a.GetLength();
    if (l1 == l2)
    {
        return std::memcmp(GetAddr(), a.GetAddr(), l1) < 0;
    }
    else
    {
        return l1 < l2;
    }
}

bool IPAddress::operator <=(const IPAddress& a) const
{
    SOCKET_LENGTH_t l1 = GetLength();
    SOCKET_LENGTH_t l2 = a.GetLength();
    if (l1 == l2)
    {
        return std::memcmp(GetAddr(), a.GetAddr(), l1) <= 0;
    }
    else
    {
        return l1 < l2;
    }
}

bool IPAddress::operator >(const IPAddress& a) const
{
    SOCKET_LENGTH_t l1 = GetLength();
    SOCKET_LENGTH_t l2 = a.GetLength();
    if (l1 == l2)
    {
        return std::memcmp(GetAddr(), a.GetAddr(), l1) > 0;
    }
    else
    {
        return l1 > l2;
    }
}

bool IPAddress::operator >=(const IPAddress& a) const
{
    SOCKET_LENGTH_t l1 = GetLength();
    SOCKET_LENGTH_t l2 = a.GetLength();
    if (l1 == l2)
    {
        return std::memcmp(GetAddr(), a.GetAddr(), l1) >= 0;
    }
    else
    {
        return l1 > l2;
    }
}

SOCKET_LENGTH_t IPAddress::GetLength() const
{
    return mImpl->GetLength();
}

const void* IPAddress::GetAddr() const
{
    return mImpl->GetAddr();
}

int IPAddress::GetAddressFamily() const
{
    return mImpl->GetAddressFamily();
}

void IPAddress::Init(IPAddressImpl* pImpl)
{
    mImpl->Release();
    mImpl = pImpl;
}

bool IPAddress::Parse(const std::string& addr, IPAddress& result)
{
    IPAddressImpl* pImpl = IPv4AddressImpl::Parse(addr);
    if (!pImpl)
    {
        pImpl = IPv6AddressImpl::Parse(addr);
    }

    if (pImpl)
    {
        result.Init(pImpl);
        return true;
    }
    else
    {
        return false;
    }
}

bool IPAddress::Mask(const IPAddress& Mask)
{
    IPAddressImpl* pClone = mImpl->Clone();
    mImpl->Release();
    mImpl = pClone;
    IPAddress null;
    return mImpl->Mask(Mask.mImpl, null.mImpl);
}

bool IPAddress::Mask(const IPAddress& Mask, const IPAddress& set)
{
    IPAddressImpl* pClone = mImpl->Clone();
    mImpl->Release();
    mImpl = pClone;
    return mImpl->Mask(Mask.mImpl, set.mImpl);
}

