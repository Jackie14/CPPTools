/*
 * RESTServer.cpp
 *
 */

#include "RESTServer.h"
#include "SocketAddress.h"

using namespace httpserver;

RESTServer::RESTServer(int port, const std::string &ip) :
        mPort(port), mIp(ip), mSockAddr(NULL), mServerImpl(NULL)
{
    //;
}

RESTServer::~RESTServer()
{
    if (mServerImpl)
    {
        delete mServerImpl;
    }

    if (mSockAddr)
    {
        delete mSockAddr;
    }

    for (std::map<std::string, EndPoint *>::iterator iter = mEndPoints.begin();
            iter != mEndPoints.end(); iter++)
    {
        if (iter->second)
        {
            delete iter->second;
            iter->second = NULL;
        }
    }
}

bool RESTServer::Start()
{
    // Construct mSockAddr
    if (mSockAddr)
    {
        delete mSockAddr;
        mSockAddr = NULL;
    }

    bool hasError = false;
    SocketAddress sa(mIp, mPort, hasError);
    if (!hasError && (mSockAddr = new sockaddr()))
    {
        memcpy(mSockAddr, sa.GetAddr(), sizeof(sockaddr));
    }

    if (!mSockAddr)
    {
        return false;
    }

    // Create web server
    create_webserver creater;
    creater.bind_address(mSockAddr)
            .start_method(http::http_utils::INTERNAL_SELECT)
            .max_threads(5);

    mServerImpl = new webserver(creater);

    if (!mServerImpl)
    {
        return false;
    }

    if (!ConfigEndPoints())
    {
        return false;
    }

    if (!RegisterEndPoints())
    {
        return false;
    }

    mServerImpl->start();

    return true;
}

bool RESTServer::Stop()
{
    UnregisterEndPoints();

    return mServerImpl->stop();
}

bool RESTServer::AddEndPoint(const std::string &resourceStr,
        const EndPointHandler &handler, int validatorCnt, ...)
{
    // Check Whether resourceStr exists
    std::map<std::string, EndPoint *>::iterator iter =
            mEndPoints.find(resourceStr);
    if (iter != mEndPoints.end())
    {
        return false;
    }

    // Create new EndPoint
    EndPoint *ep = new EndPoint(handler);
    if (!ep)
    {
        return false;
    }

    // Attach validators
    std::list<EndPointValidator *> validatorList;
    va_list argPtr;
    va_start(argPtr, validatorCnt);
    while (validatorCnt > 0)
    {
        EndPointValidator *validator = va_arg(argPtr, EndPointValidator *);
        if (validator)
        {
            validatorList.push_back(validator);
        }
        validatorCnt--;
    };
    va_end(argPtr);
    ep->SetValidators(validatorList);

    // Add EndPoint
    mEndPoints[resourceStr] = ep;

    return true;
}

bool RESTServer::RegisterEndPoints()
{
    for (std::map<std::string, EndPoint *>::iterator iter = mEndPoints.begin();
            iter != mEndPoints.end(); iter++)
    {
        if (!mServerImpl->register_resource(iter->first, iter->second, true))
        {
            return false;
        }
    }

    return true;
}

void RESTServer::UnregisterEndPoints()
{
    for (std::map<std::string, EndPoint *>::iterator iter = mEndPoints.begin();
            iter != mEndPoints.end(); iter++)
    {
        mServerImpl->unregister_resource(iter->first);
    }
}
