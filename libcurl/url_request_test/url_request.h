#ifndef URL_REQUEST_H_
#define URL_REQUEST_H_

#include "curl/curl.h"

namespace mereboard {

template<typename T>
class HttpClient {
public:
    bool debug_;
    HttpClient() {}
    ~HttpClient() {}

    bool URLRequest(const char* url);

    bool GetFile(const char* url, const char* file_name, T* t = NULL);
    bool PutFile(const char* url, const char* file_name, T* t = NULL);
private:
    static size_t ReadFileFunc(void* ptr, size_t size, size_t nmemb, void *userdata);
    static size_t WriteFileFunc(void* buffer, size_t size, size_t nmemb, void *userdata);

    static int PutFileProgressFunc(void* userdata,
        double dltotal,
        double dlnow,
        double ultotal, //上传:总大小
        double ulnow    //上传:当前大小
        );

    static int GetFileProgressFunc(void   *userdata,
        double dltotal,  //下载:当前大小
        double dlnow,    //下载:当前大小
        double ultotal,
        double ulnow);
};

template<typename T>
bool HttpClient<T>::GetFile(const char* url, const char* file_name, T* t = NULL) {
    CURL* curl;
    CURLcode res = CURL_LAST;

    curl = curl_easy_init();
    if (curl == NULL) {
        return false;
    }

    //设置远程URL
    curl_easy_setopt(curl, CURLOPT_URL, url);

    //设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpClient::WriteFileFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, t);
    if (t !=NULL){
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, HttpClient::GetFileProgressFunc);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, t);
    }

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return false;
    }

    return true;
}

template<typename T>
size_t HttpClient<T>::WriteFileFunc(void *buffer, size_t size, size_t nmemb, void *userdata) {
    T* stream = (T*)userdata;

    size_t nb_bytes = size * nmemb;

    printf("recv buffer size = %d\n", nb_bytes);

    Sleep(1000);

    return nb_bytes;
}

template<typename T>
int HttpClient<T>::GetFileProgressFunc(void *userdata,
                                       double dltotal,  //下载:总大小
                                       double dlnow,    //下载:当前大小
                                       double ultotal,
                                       double ulnow)

{
    T *pT = (T*)userdata;

    printf("get file progress, total = %f, now = %f\n", dltotal, dlnow);

    Sleep(1000);

    return 0;
}

}

#endif