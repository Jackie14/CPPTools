/*
 * EndPoint.cpp
 *
 */

#include "RESTEndPoint.h"

EndPointHandler EndPointHandler::mDefaultHandler;

EndPointHandler::EndPointHandler()
{
    // No allowed method, by default
}

EndPointHandler::~EndPointHandler()
{
    //;
}

const http_response EndPointHandler::handle_GET(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_POST(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_PUT(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_DELETE(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_HEAD(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_TRACE(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_OPTIONS(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

const http_response EndPointHandler::handle_CONNECT(const http_request &req, const EndPoint &ep) const
{
    return details::empty_render(req);
}

void EndPointHandler::AddAllowedMethod(const std::string &method)
{
    mAllowedMethod.push_back(method);
}

void EndPointHandler::DeleteAllowedMethod(const std::string &method)
{
    mAllowedMethod.remove(method);
}

const std::list<std::string> &EndPointHandler::GetAllowedMethods() const
{
    return mAllowedMethod;
}

EndPoint::EndPoint(const EndPointHandler &handler) : mEpHandler(handler)
{
    // Configure allowed Methods
    disallow_all();
    for (std::list<std::string>::const_iterator citer = mEpHandler.GetAllowedMethods().begin();
            citer != mEpHandler.GetAllowedMethods().end();
            citer++)
    {
        set_allowing(*citer, true);
    }
}

EndPoint::~EndPoint()
{
    //;
}

void EndPoint::SetValidators(const std::list<EndPointValidator *> &validators)
{
    mEpValidators = validators;
}

#define RENDER_TEMPLET(method) \
http_response_builder failedResp("");\
if (!Validate(req, *this, failedResp)) \
{\
    return failedResp;\
}\
return mEpHandler.handle_##method(req, *this);

const http_response EndPoint::render_GET(const http_request& req)
{
    RENDER_TEMPLET(GET)
}

const http_response EndPoint::render_POST(const http_request& req)
{
    RENDER_TEMPLET(POST)
}

const http_response EndPoint::render_PUT(const http_request& req)
{
    RENDER_TEMPLET(PUT)
}

const http_response EndPoint::render_DELETE(const http_request& req)
{
    RENDER_TEMPLET(DELETE)
}

const http_response EndPoint::render_HEAD(const http_request& req)
{
    RENDER_TEMPLET(HEAD)
}

const http_response EndPoint::render_TRACE(const http_request& req)
{
    RENDER_TEMPLET(TRACE)
}

const http_response EndPoint::render_OPTIONS(const http_request& req)
{
    RENDER_TEMPLET(OPTIONS)
}

const http_response EndPoint::render_CONNECT(const http_request& req)
{
    RENDER_TEMPLET(CONNECT)
}

bool EndPoint::Validate(const http_request& req, const EndPoint &ep,
        http_response_builder& failedResp) const
{
    for (std::list<EndPointValidator *>::const_iterator citer = mEpValidators.begin();
            citer != mEpValidators.end();
            citer++)
    {
        if (*citer == NULL)
        {
            continue;
        }

        EndPointValidator &validator = **citer;

        if (!validator.Validate(req, ep, failedResp))
        {
            return false;
        }
    }

    return true;
}
