//////////////////////////////////////////////////////////////////////////
// HTTPMessage.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef HTTPMessage_INCLUDED
#define HTTPMessage_INCLUDED

#include <string>
#include <map>
#include "Socket.h"

class HTTPMessage
{
public:
    HTTPMessage();
    HTTPMessage(int code, const std::string& response);
    virtual ~HTTPMessage();

    // Check if the HTTP message is validly formed
    bool IsValid() const;

    // Set the HTTP response information
    bool SetResponse(int code, const std::string& response);

    // Add header
    bool AddHeader(const std::string& key, const std::string& value);

    // Clear Headers
    void CleanHeaders();

    // Set the HTTP message body
    bool SetBody(const std::string& body);

    // Send the completed HTTP message
    bool Send(Socket& streamSock);

private:
    // Response string and code supplied on the HTTP status line
    int mResponseCode;
    std::string mResponseString;

    // Header field/value pairs
    std::map<std::string, std::string> mHeaders;

    // Body of the message
    std::string mBodyString;
};

#endif // HTTPMessage_INCLUDED
