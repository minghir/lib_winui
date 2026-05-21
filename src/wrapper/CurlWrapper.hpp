#ifndef CURLWRAPPER_HPP
#define CURLWRAPPER_HPP

#include <string>
#define CURL_STATICLIB
#include <curl\curl.h>

class CurlWrapper {
private:
    CURL* curl;
    std::string userAgent;

    // Funcție statică pentru callback-ul de scriere
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    CurlWrapper();
    ~CurlWrapper();

    // Setează credențialele pentru FTP/HTTP Auth
    void setCredentials(const std::string& user, const std::string& pass);

    // Metodă generică de download
    bool downloadFile(const std::string& url, const std::string& localPath, bool isFtp = false);

    // Poți adăuga și metode pentru GET/POST dacă vei avea nevoie de API-uri REST
};

#endif