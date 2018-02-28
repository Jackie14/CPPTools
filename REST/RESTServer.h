/*
 * RESTServer.h
 *
 */

#ifndef RESTSERVER_H_
#define RESTSERVER_H_

#include <map>
#include <string>
#include "RESTEndPoint.h"

class RESTServer
{
public:
    RESTServer(int port, const std::string &ip = "0.0.0.0");
    virtual ~RESTServer();

    bool Start();
    bool Stop();

protected:
    // Override this function to Configure EndPoints for current REST server
    virtual bool ConfigEndPoints() = 0;

    bool AddEndPoint(const std::string &resourceStr, const EndPointHandler &handler,
            int validatorCnt, ...);

private:
    bool RegisterEndPoints();
    void UnregisterEndPoints();

private:
    int mPort;
    std::string mIp;
    sockaddr *mSockAddr;
    webserver *mServerImpl;
    std::map<std::string, EndPoint *> mEndPoints;
};


#endif /* RESTSERVER_H_ */
