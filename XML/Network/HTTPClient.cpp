/*
 * HTTPClient.cpp
 *
 */

#include <vector>
#include <cstdlib>
#include <curl/curl.h>
#include "StringUtilities.h"
#include "HTTPClient.h"

bool HTTPClient::Access()
{
    std::string respBody;
    return Access(respBody);
}

bool HTTPClient::Access(std::string &respBody)
{
    std::map<std::string, std::string> respheaders;
    return Access(respBody, respheaders);
}


bool HTTPClient::Access(std::string &respBody,
        std::map<std::string, std::string>& respheaders)
{
    std::string dstIpStr, errMsg;
    int totalTime = 0;
    return Access(respBody, respheaders, dstIpStr, totalTime, errMsg);
}

bool HTTPClient::Access(std::string &respBody,
        std::map<std::string, std::string>& respheaders,
        std::string &dstIpStr, int &duration, std::string &errMsg)
{
    // Initialize global curl.
    static bool curlGlobalInited = false;
    if (!curlGlobalInited)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        curlGlobalInited = true;
    }

    // Initialize a new curl handle
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        return false;
    }

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, mUrl.c_str());
    // Set Method
    if (!mMethod.empty())
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, mMethod.c_str());
    }
    // Set body
    if (!mBody.empty())
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mBody.c_str());
    }
    // Set user & password
    if (mUser != "" && mPassword != "")
    {
        curl_easy_setopt(curl, CURLOPT_USERPWD,
                (mUser + ":" + mPassword).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, mTimeout);

    // Set HTTP request header
    struct curl_slist *headerList = NULL;

    for (std::list<std::string>::const_iterator header = mHeaders.begin();
            header != mHeaders.end(); header++)
    {
        headerList = curl_slist_append(headerList, header->c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    // Enable all Accept-Encoding functions. otherwise libcurl will not decode response automatically
    curl_easy_setopt(curl, CURLOPT_ENCODING, "");

    // Do not verify SSL Certificate currently.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // Set callback and buff for response status line and header
    std::string respHeaderData;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, ResponseCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &respHeaderData);

    // Set callback and buff for response body
    respBody.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respBody);

    if (mUseSystemProxySettings)
    {
        // Use system PROXY setting. Curl will get PROXY setting from environment variables
        // 1) https_proxy=user:password@<proxy server IP>:<port>
        // 2) http_proxy=user:password@<proxy server IP>:<port>
    }
    else
    {
        // Set PROXY, when do not want to use system PROXY settings
        if (mEnableProxy)
        {
            std::string enforcedHost = mProxyHost;
            if (enforcedHost == "")
            {
                // Empty ProxyHost is an invalid configure, while AccessBackendViaProxy is enabled.
                // We replace this empty host with a INVALID host string, which will make PROXY accessing failed.
                // If we leave it as an empty host, curl will access remote server directly --- without PROXY,
                // which should not be the user's intention
                enforcedHost = "Proxy Host Is Not Configured";
            }

            curl_easy_setopt(curl, CURLOPT_PROXY, enforcedHost.c_str());
            curl_easy_setopt(curl, CURLOPT_PROXYPORT, mProxyPort);

            if (mProxyUser != "" && mProxyPassword != "")
            {
                curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
                        (mProxyUser + ":" + mProxyPassword).c_str());
            }
        }
        else
        {
            // Disable PROXY
            curl_easy_setopt(curl, CURLOPT_PROXY, "");
        }
    }

    // Used for debug
    curl_easy_setopt(curl, CURLOPT_VERBOSE, mVerbose);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, mNoProgress);

    // Access the URL
    CURLcode performCode = curl_easy_perform(curl);

    // Get info of previous finished http request
    char *dstIp = NULL;
    curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &dstIp);
    dstIpStr = dstIp;

    double totalTime;
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
    duration = int(totalTime);

    // Release resources
    curl_easy_cleanup(curl);
    curl_slist_free_all(headerList);

    // Return true/false accordingly
    std::string version = "";
    int statusCode = 0;
    std::string statusString = "";

    std::map<std::string, std::string> headerMap;
    if (performCode == CURLE_OK &&
            ParseResponseMetaData(respHeaderData, version, statusCode, statusString, headerMap) &&
            statusCode == 200)
    {
        return true;
    }
    else
    {
        errMsg = curl_easy_strerror(performCode);
        return false;
    }
}

// Callback used for curl option: CURLOPT_WRITEFUNCTION and CURLOPT_HEADERFUNCTION
size_t HTTPClient::ResponseCallback(void *buf, size_t size, size_t n, void *userBuf)
{
    size_t realsize = (size * n);
    std::string *userData = (std::string *)userBuf;

    if (userData && buf)
    {
        (*userData) += std::string((char *)buf, realsize);
    }

    return realsize;
}

// Parse response metaData (<status line, header>).
// NOTE: The metaData may contain more then one response data. Only need to parse the last one
bool HTTPClient::ParseResponseMetaData(const std::string& metaData,
        std::string& version,
        int& statusCode,
        std::string& statusStr,
        std::map<std::string, std::string>& headerMap)
{
    const std::string CRLF = "\r\n";
    const std::string doubleCRLF = "\r\n\r\n";

    // Get the last response metaData.
    // NOTE: Since the input metaData may contain more then one response data.
    //      Currently, only need to parse the last one.
    size_t lastMetaEnd = metaData.rfind(doubleCRLF);
    if (lastMetaEnd == std::string::npos)
    {
        return false;
    }

    size_t lastMetaBegin = metaData.rfind(doubleCRLF, lastMetaEnd - doubleCRLF.length());
    if (lastMetaBegin == std::string::npos)
    {
        // Maybe there is only one response meta data
        lastMetaBegin = 0;
    }
    else
    {
        lastMetaBegin += doubleCRLF.length();
    }

    std::string lastMeta = metaData.substr(lastMetaBegin, lastMetaEnd - lastMetaBegin);

    // Get status line
    size_t statusLineBegin = 0;
    size_t statusLineEnd = lastMeta.find(CRLF);
    if (statusLineEnd == std::string::npos)
    {
        return false;
    }

    std::string statusLine = lastMeta.substr(statusLineBegin, statusLineEnd - statusLineBegin);

    // Parse status line
    std::vector<std::string> splitResult;
    if (!StringUtilities::SplitString(statusLine, " ", true, splitResult) ||
            splitResult.size() < 3)
    {
        return false;
    }

    version = splitResult[0];
    statusCode = atoi(splitResult[1].c_str());
    statusStr = splitResult[2];

    for (size_t i = 3; i < splitResult.size(); i++)
    {
        statusStr += " " + splitResult[i];
    }

    // Get and parse header
    std::string header = lastMeta.substr(statusLineEnd + CRLF.length());

    headerMap.clear();
    size_t fieldStart = 0;
    size_t fieldEnd = 0;

    while (fieldStart < header.length())
    {
        fieldEnd = header.find(CRLF, fieldStart);
        if (fieldEnd == std::string::npos)
        {
            // Maybe it is the last header
            fieldEnd = header.length();
        }
        else if (fieldEnd == fieldStart)
        {
            // Empty header (maybe \r\n\r\n), go for next header
            fieldStart = fieldEnd + CRLF.length();
            continue;
        }

        std::string field = header.substr(fieldStart, fieldEnd - fieldStart);

        size_t pos = field.find(":");
        if (pos != std::string::npos)
        {
            std::string key = field.substr(0, pos);
            std::string val = field.substr(pos + 2, (field.length() - (pos + 2))); // 2 is for ": ".
            headerMap.insert(std::map<std::string, std::string>::value_type(key, val));
        }

        fieldStart = fieldEnd + CRLF.length();
    }

    return true;
}
