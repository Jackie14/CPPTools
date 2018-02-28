//////////////////////////////////////////////////////////////////////////
// DNS.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "DNS.h"
#include <assert.h>
#include <string.h>
#include "SocketAddress.h"
#include "NumberFormatter.h"

bool DNS::HostByName(const std::string& hostname, HostEntry& hostEntry)
{
    addrinfo* pAI;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
    int rc = getaddrinfo(hostname.c_str(), NULL, &hints, &pAI);
    if (rc == 0)
    {
        hostEntry = HostEntry(pAI);
        freeaddrinfo(pAI);
        return true;
    }
    else
    {
        return false;
    }
}

bool DNS::HostByAddress(const IPAddress& address, HostEntry& hostEntry)
{
    bool hasError = false;
    SocketAddress sa(address, 0, hasError);
    if(hasError)
    {
        return false;
    }

    static char fqname[1024];
    int rc = getnameinfo(sa.GetAddr(), sa.GetLength(), fqname,
            sizeof(fqname), NULL, 0, NI_NAMEREQD);
    if (rc == 0)
    {
        addrinfo* pAI;
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
        rc = getaddrinfo(fqname, NULL, &hints, &pAI);
        if (rc == 0)
        {
            hostEntry = HostEntry(pAI);
            freeaddrinfo(pAI);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool DNS::Resolve(const std::string& address, HostEntry& hostEntry)
{
    IPAddress ip;
    if (IPAddress::Parse(address, ip))
    {
        return HostByAddress(ip, hostEntry);
    }
    else
    {
        return HostByName(address, hostEntry);
    }
}

bool DNS::ResolveOne(const std::string& address, IPAddress& ipAddr)
{
    HostEntry entry;
    bool ret = Resolve(address, entry);
    if (ret && !entry.GetAddresses().empty())
    {
        ipAddr = entry.GetAddresses()[0];
        return true;
    }
    else
    {
        return false;
    }
}

bool DNS::ThisHost(HostEntry& hostEntry)
{
    std::string hostName = HostName();
    if(hostName.size() <= 0)
    {
        return false;
    }

    return HostByName(hostName, hostEntry);
}

std::string DNS::HostName()
{
    char buffer[256];
    int rc = gethostname(buffer, sizeof(buffer));
    if (rc == 0)
    {
        return std::string(buffer);
    }
    else
    {
        return "";
    }
}
