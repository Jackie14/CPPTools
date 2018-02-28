//////////////////////////////////////////////////////////////////////////
// PingUtilities.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "PingUtilities.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include "Log.h"


#define DEFAULT_DATA_LEN  (64-ICMP_MINLEN)// default data length
#define MAX_IP_LEN    60
#define MAX_ICMP_LEN  76
#define MAX_PACKET_LEN   (65536 - 60 - ICMP_MINLEN) // max packet size

bool PingUtilities::Ping(const std::string& target, int maxCount)
{
    if(target.size() <= 0)
    {
        // Invalid target
        return false;
    }

    hostent* hostEnt = NULL;
    std::string hostName;
    sockaddr_in toAddr;
    toAddr.sin_family = AF_INET;
    // Try to convert as dotted decimal address,
    // else if that fails assume it's a hostname
    toAddr.sin_addr.s_addr = inet_addr(target.c_str());
    if (toAddr.sin_addr.s_addr != (unsigned int)-1)
    {
        hostName = target;
    }
    else
    {
        hostEnt = gethostbyname(target.c_str());
        if (!hostEnt)
        {
            // Unknown host
            return false;
        }
        toAddr.sin_family = hostEnt->h_addrtype;
        bcopy(hostEnt->h_addr, (caddr_t)&toAddr.sin_addr, hostEnt->h_length);
        char hostNameBuf[MAXHOSTNAMELEN];
        strncpy(hostNameBuf, hostEnt->h_name, sizeof(hostNameBuf) - 1);
        hostName = hostNameBuf;
    }

    int sock;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        // Failed to create RAW socket
        return false;
    }

    unsigned char outPack[MAX_PACKET_LEN];
    const int icmpSeq = 12345;
    icmp* icmpHdr = (icmp*)outPack;
    icmpHdr->icmp_type = ICMP_ECHO;
    icmpHdr->icmp_code = 0;
    icmpHdr->icmp_cksum = 0;
    icmpHdr->icmp_seq = icmpSeq; // seq and id must be reflected
    icmpHdr->icmp_id = getpid();

    int cc = DEFAULT_DATA_LEN + ICMP_MINLEN;
    icmpHdr->icmp_cksum = CalChecksum((unsigned short*)icmpHdr, cc);

    // Timeout for 1 second
    timeval timeoutVal;
    timeoutVal.tv_sec = 1;
    timeoutVal.tv_usec = 0;

    bool isPingable = false;
    int count = 0;
    while (!isPingable && (count < maxCount))
    {
        count++;
        LOG(LogDebug, "Ping %s, %d time", hostName.c_str(), count);

        // Time to send
        timeval start;
        gettimeofday(&start, NULL);

        // Send ping packet
        int ret = sendto(sock, (char*)outPack, cc, 0, (sockaddr*)&toAddr,
                (socklen_t)sizeof(sockaddr_in));
        if (ret < 0 || ret != cc)
        {
            // Sendto error
            continue;
        }

        // Watch stdin (fd 0) to see when it has input.
        fd_set fdSets;
        FD_ZERO(&fdSets);
        FD_SET(sock, &fdSets);
        // Wait up to one seconds.
        ret = select(sock + 1, &fdSets, NULL, NULL, &timeoutVal);
        if (ret == -1)
        {
            // Select error
            continue;
        }
        else if(ret)
        {
            int fromlen = sizeof(sockaddr_in);
            sockaddr_in from;
            int packetLen = DEFAULT_DATA_LEN + MAX_IP_LEN + MAX_ICMP_LEN;
            unsigned char packet[DEFAULT_DATA_LEN + MAX_IP_LEN + MAX_ICMP_LEN];
            bzero(packet, packetLen);
            if ((ret = recvfrom(sock, (char*)packet, packetLen, 0,
                    (sockaddr*)&from, (socklen_t*)&fromlen)) < 0)
            {
                // recvfrom error
                continue;
            }

            // Check the IP header
            //ip* ipHdr = (ip*)((char*)packet);
            int ipHdrLen = sizeof(ip);
            if (ret < (ipHdrLen + ICMP_MINLEN))
            {
                // Invalid response packet
                continue;
            }

            // Now the ICMP part
            icmpHdr = (icmp*)(packet + ipHdrLen);
            if (icmpHdr->icmp_type == ICMP_ECHOREPLY)
            {
                // Get echo reply
                if (icmpHdr->icmp_seq != icmpSeq)
                {
                    // Illegal sequence
                    continue;
                }
                if (icmpHdr->icmp_id != getpid())
                {
                    // Illegal id
                    continue;
                }

                // Done
                isPingable = true;
            }
            else
            {
                // Not an echo reply
                continue;
            }

            // Time get response
            timeval end;
            gettimeofday(&end, NULL);
            // Convert to ms
            int duration = (1000000 * (end.tv_sec - start.tv_sec)
                    + (end.tv_usec - start.tv_usec))/1000;
            if (duration < 1)
            {
                duration = 1;
            }
            LOG(LogDebug, "Ping elapsed time: %d ms", duration);
        }
        else
        {
            // No data within one seconds
            continue;
        }
    }

    return isPingable;
}

unsigned short PingUtilities::CalChecksum(unsigned short* addr,
        unsigned int len)
{
    unsigned short answer = 0;
    // Our algorithm is simple, using a 32 bit accumulator (sum), we add
    // sequential 16 bit words to it, and at the end, fold back all the
    // carry bits from the top 16 bits into the lower 16 bits.
    unsigned int sum = 0;
    while (len > 1)
    {
        sum += *addr++;
        len -= 2;
    }

    // Mop up an odd byte, if necessary
    if (len == 1)
    {
        *(unsigned char *)&answer = *(unsigned char *)addr ;
        sum += answer;
    }

    // Add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add high 16 to low 16
    sum += (sum >> 16); // add carry
    answer = ~sum; // truncate to 16 bits
    return answer;
}
