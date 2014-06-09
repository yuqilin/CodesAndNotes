#include "url_request.h"


#pragma comment(lib, "libcurl.lib")

class HttpClientTest : public mereboard::HttpClient<HttpClientTest>
{
public:
    HttpClientTest() {}
    ~HttpClientTest() {}
};

int main()
{
    HttpClientTest test;
    test.GetFile("http://58.59.19.25/bdcdn.p2sp.baidu.com/bdcdn/C48A31F90E5DF4CB1E6F325A85ED400B?wsiphost=ipdbm",
        "D:\\urlfetcher\\urlfetcher.tmp", &test);

    return 0;
}