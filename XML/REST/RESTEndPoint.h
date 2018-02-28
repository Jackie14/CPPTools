/*
 * EndPoint.h
 *
 */

#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <list>
#include <string>
#include <httpserver.hpp>
using namespace httpserver; // Fix me: use namespace in header file is not good

class EndPoint;

class EndPointHandler
{
public:
    EndPointHandler();
    virtual ~EndPointHandler();

    // Override following handler method, if you want
    virtual const http_response handle_GET(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_POST(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_PUT(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_DELETE(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_HEAD(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_TRACE(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_OPTIONS(const http_request &req, const EndPoint &ep) const;
    virtual const http_response handle_CONNECT(const http_request &req, const EndPoint &ep) const;

    void AddAllowedMethod(const std::string &method);
    void DeleteAllowedMethod(const std::string &method);
    const std::list<std::string> &GetAllowedMethods() const;

public:
    static EndPointHandler mDefaultHandler;

private:
    std::list<std::string> mAllowedMethod;
};

class EndPointValidator
{
public:
    virtual ~EndPointValidator() {}

    virtual bool Validate(const http_request& req, const EndPoint &ep,
            http_response_builder& failedResp) const = 0;
};

class EndPoint : public http_resource
{
public:
    EndPoint(const EndPointHandler &handler = EndPointHandler::mDefaultHandler);

    virtual ~EndPoint();

    void SetValidators(const std::list<EndPointValidator *> &validators);

private:
    virtual const http_response render_GET(const http_request& req);
    virtual const http_response render_POST(const http_request& req);
    virtual const http_response render_PUT(const http_request& req);
    virtual const http_response render_DELETE(const http_request& req);
    virtual const http_response render_HEAD(const http_request& req);
    virtual const http_response render_TRACE(const http_request& req);
    virtual const http_response render_OPTIONS(const http_request& req);
    virtual const http_response render_CONNECT(const http_request& req);

    bool Validate(const http_request& req, const EndPoint &ep,
            http_response_builder& failedResp) const;

private:
    const EndPointHandler &mEpHandler;
    std::list<EndPointValidator *> mEpValidators;
};

#endif /* ENDPOINT_H_ */
