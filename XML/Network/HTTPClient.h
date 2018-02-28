/*
 * HTTPClient.h
 *
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <list>
#include <map>
#include <string>

class HTTPClient
{
public:
    HTTPClient() :
        mTimeout(0), mEnableProxy(false),
        mProxyPort(0), mUseSystemProxySettings(false),
        mVerbose(0), mNoProgress(1){};

    HTTPClient &SetUrl(const std::string &url)
    {
        mUrl = url;
        return *this;
    }
    HTTPClient &SetMethod(const std::string &method)
    {
        mMethod = method;
        return *this;
    }
    HTTPClient &SetBody(const std::string &body)
    {
        mBody = body;
        return *this;
    }
    HTTPClient &SetUsrPwd(const std::string &usr, const std::string &pwd)
    {
        mUser = usr;
        mPassword = pwd;
        return *this;
    }
    HTTPClient &SetTimeout(int timeout)
    {
        mTimeout = timeout;
        return *this;
    }
    HTTPClient &AddHeader(const std::string &header)
    {
        mHeaders.push_back(header);
        return *this;
    }
    HTTPClient &AddHeader(const std::list<std::string> &headers)
    {
        mHeaders.insert(mHeaders.end(), headers.begin(), headers.end());
        return *this;
    }
    HTTPClient &AddHeader(const std::map<std::string, std::string> &headers)
    {
        for (std::map<std::string, std::string>::const_iterator header = headers.begin();
                header != headers.end(); header++)
        {
            mHeaders.push_back((header->first + ": " + header->second).c_str());
        }
        return *this;
    }
    HTTPClient &EnableProxy(bool enable)
    {
        mEnableProxy = enable;
        return *this;
    }
    HTTPClient &SetProxyAddr(const std::string &host, int port)
    {
        mProxyHost = host;
        mProxyPort = port;
        return *this;
    }
    HTTPClient &SetProxyUsrPwd(const std::string &usr, const std::string &pwd)
    {
        mProxyUser = usr;
        mProxyPassword = pwd;
        return *this;
    }
    HTTPClient &UseSystemProxySettings(bool use)
    {
        mUseSystemProxySettings = use;
        return *this;
    }
    HTTPClient &SetDebug(int verbose, bool noProgress)
    {
        mVerbose = verbose;
        mNoProgress = noProgress;
        return *this;
    }

    bool Access();
    bool Access(std::string &respBody);
    bool Access(std::string &respBody, std::map<std::string, std::string>& respheaders);
    bool Access(std::string &respBody, std::map<std::string, std::string>& respheaders,
            std::string &dstIpStr, int &duration, std::string &errMsg);

private:
    static size_t ResponseCallback(void *buf, size_t size, size_t n, void *userBuf);
    static bool ParseResponseMetaData(const std::string& metaData,
            std::string& version,
            int& statusCode,
            std::string& statusStr,
            std::map<std::string, std::string>& headerMap);

private:
    // Request related
    std::string mUrl;
    std::string mMethod;
    std::list<std::string> mHeaders;
    std::string mBody;
    std::string mUser;
    std::string mPassword;
    int mTimeout;

    // Proxy related
    bool mEnableProxy;
    std::string mProxyHost;
    int mProxyPort;
    std::string mProxyUser;
    std::string mProxyPassword;
    bool mUseSystemProxySettings;

    // Debug related
    int mVerbose;
    int mNoProgress;
};

#endif /* HTTPCLIENT_H_ */
