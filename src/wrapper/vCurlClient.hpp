#ifndef V_CURL_CLIENT_HPP
#define V_CURL_CLIENT_HPP

#include <string>
#include <curl/curl.h>
#include "../ui/ConsoleManager.hpp"

class vCurlClient {
private:
    CURL* curl;
    std::string lastError;

    // Callback static pentru a redirecționa datele către un stream de fișier
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    vCurlClient();
    ~vCurlClient();

    // Dezactivăm copierea pentru a evita problemele cu pointerul CURL
    vCurlClient(const vCurlClient&) = delete;
    vCurlClient& operator=(const vCurlClient&) = delete;

    // Metoda principală pentru download (HTTP sau FTP)
    bool download(const std::string& url, const std::string& localFilePath, const std::string& userPwd = "");

    std::string getLastError() const { return lastError; }
};

#endif