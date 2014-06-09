#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

#include <string>

using std::string;

class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();
    static int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
    static int Get(const std::string & strUrl, std::string & strResponse);
};

enum RequestType {
    RequestTypeGet,
    RequestTypePost,
};

class HttpClientDelegate
{
public:
    virtual void OnResponseCompleted(void* user) = 0;
    virtual void OnResponseProgress(void* user) = 0;
};

class HttpClient {
public:
    int AddRequest(const std::string & strUrl,
        RequestType type,
        HttpClientDelegate* delegate);
    
}
#endif