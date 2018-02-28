//////////////////////////////////////////////////////////////////////////
// HTTPMessage.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "HTTPMessage.h"
#include "StringUtilities.h"
#include <iostream>

HTTPMessage::HTTPMessage()
{
    mResponseCode = 0;
    mResponseString.clear();
    mHeaders.clear();
    mBodyString.clear();
}

HTTPMessage::HTTPMessage(int code, const std::string& response)
{
    mResponseCode = code;
    mResponseString = response;
    mHeaders.clear();
    mBodyString.clear();
}

HTTPMessage::~HTTPMessage()
{
}

// Check if the HTTP message is validly formed
bool HTTPMessage::IsValid() const
{
    if (mResponseString.size() <= 0 || mResponseCode < 1 || mResponseCode > 999)
    {
        return false;
    }

    return true;
}

// Set the HTTP response information
bool HTTPMessage::SetResponse(int code, const std::string& response)
{
    if (response.size() <= 0 || code < 1 || code > 999)
    {
        return false;
    }

    mResponseCode = code;
    mResponseString = response;

    return true;
}

// Add header
bool HTTPMessage::AddHeader(const std::string& key, const std::string& value)
{
    if (key.size() <= 0 || value.size() <= 0)
    {
        return false;
    }

    mHeaders.insert(std::pair<std::string, std::string>(key, value));

    return true;
}

// Clear Headers
void HTTPMessage::CleanHeaders()
{
    mHeaders.clear();
}

// Set the HTTP message body
bool HTTPMessage::SetBody(const std::string& body)
{
    if (body.size() <= 0)
    {
        return false;
    }

    mBodyString = body;

    return true;
}

// Send the completed HTTP message
bool HTTPMessage::Send(Socket& streamSock)
{
    if (!IsValid())
    {
        return false;
    }
    if (streamSock.Sockfd() < 0)
    {
        return false;
    }

    // The whole header string to send
    std::string headerStr;

    // Add the response line
    std::string message;
    message = StringUtilities::FormatString("HTTP/1.1 %d %s\r\n",
            mResponseCode, mResponseString.c_str());
    headerStr += message;

    // Add the headers
    for (std::map<std::string, std::string>::const_iterator citer =
            mHeaders.begin(); citer != mHeaders.end(); ++citer)
    {
        std::string key = citer->first;
        std::string value = citer->second;
        message = StringUtilities::FormatString("%s: %s\r\n", key.c_str(),
                value.c_str());
        headerStr += message;
    }

    // Add the date
    time_t currentTime = time(NULL);
    char timeBuf[30];
    strftime(timeBuf, sizeof(timeBuf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(
            &currentTime));
    message = StringUtilities::FormatString("Date: %s\r\n", timeBuf);
    headerStr += message;

    // Add the Content-length
    message = StringUtilities::FormatString("Content-length: %u\r\n",
            mBodyString.size());
    headerStr += message;

    // And the final blank line to signify the end of the headers
    headerStr += "\r\n";

    int len = streamSock.SendData(headerStr.c_str(), headerStr.size());
    if(len < 0)
    {
        return false;
    }

    // Send the body if present
    if (mBodyString.size() > 0)
    {
        len = streamSock.SendData(mBodyString.c_str(), mBodyString.size());
    }

    if(len < 0)
    {
        return false;
    }

    return true;
}
