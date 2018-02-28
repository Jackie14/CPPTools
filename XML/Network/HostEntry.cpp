//////////////////////////////////////////////////////////////////////////
// HostEntry.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "HostEntry.h"
#include "Log.h"
#include <algorithm>

HostEntry::HostEntry()
{
}

HostEntry::HostEntry(hostent* entry)
{
    mName = entry->h_name;
    char** alias = entry->h_aliases;
    if (alias)
    {
        while (*alias)
        {
            mAliases.push_back(std::string(*alias));
            ++alias;
        }
    }
    char** address = entry->h_addr_list;
    if (address)
    {
        while (*address)
        {
            bool hasError = false;
            mAddresses.push_back(IPAddress(*address, entry->h_length, hasError));
            ++address;
        }
    }
}

HostEntry::HostEntry(addrinfo* ainfo)
{
    if(!ainfo)
    {
        return;
    }

    for (addrinfo* ai = ainfo; ai; ai = ai->ai_next)
    {
        bool hasError = false;
        if (ai->ai_canonname)
        {
            mName = ai->ai_canonname;
        }
        if (ai->ai_addrlen && ai->ai_addr)
        {
            switch (ai->ai_addr->sa_family)
            {
            case AF_INET:
                mAddresses.push_back(IPAddress(&reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_addr,
                        sizeof(in_addr), hasError));
                break;
            case AF_INET6:
                mAddresses.push_back(IPAddress(&reinterpret_cast<sockaddr_in6*>(ai->ai_addr)->sin6_addr, sizeof(in6_addr),
                        reinterpret_cast<sockaddr_in6*>(ai->ai_addr)->sin6_scope_id,
                        hasError));
                break;
            default:
                break;
            }
        }
    }
}

HostEntry::HostEntry(const HostEntry& entry) :
    mName(entry.mName), mAliases(entry.mAliases), mAddresses(entry.mAddresses)
{
}

HostEntry& HostEntry::operator =(const HostEntry& entry)
{
    if (&entry != this)
    {
        mName = entry.mName;
        mAliases = entry.mAliases;
        mAddresses = entry.mAddresses;
    }
    return *this;
}

HostEntry::~HostEntry()
{
}
