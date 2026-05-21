#define CURL_STATICLIB //
#include <winsock2.h> // Adaugă asta dacă ești pe Windows, înainte de curl.h
#include <curl/curl.h>
#include <cstdio>

#include "vCurlClient.hpp"

// Workaround pentru identificator lipsă în headerele locale
#ifndef CURLOPT_FTP_USE_PASV
#define CURLOPT_FTP_USE_PASV (CURLoption)85
#endif


vCurlClient::vCurlClient() {
    curl = curl_easy_init();
    if (!curl) {
        LOG_ERROR(L"CURL: Eroare la inițializarea easy_handle!");
    }
}

vCurlClient::~vCurlClient() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

size_t vCurlClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    FILE* fp = static_cast<FILE*>(userp);
    return fwrite(contents, size, nmemb, fp);
}

bool vCurlClient::download(const std::string& url, const std::string& localFilePath, const std::string& userPwd) {
    if (!curl) return false;

    FILE* fp;
    if (fopen_s(&fp, localFilePath.c_str(), "wb") != 0) {
        LOG_ERROR(L"CURL: Nu am putut crea fișierul local: " + std::wstring(localFilePath.begin(), localFilePath.end()));
        return false;
    }

    // Resetăm opțiunile pentru a curăța setările de la apelul anterior
    curl_easy_reset(curl);

    // Setări de bază
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Pentru redirect-uri HTTP

    // Setări specifice FTP
    if (url.compare(0, 6, "ftp://") == 0) {
        curl_easy_setopt(curl, CURLOPT_FTP_USE_PASV, 1L); // Mod pasiv obligatoriu
        // Folosim constanta direct din enum-ul libcurl dacă identificatorul macro are probleme
        //curl_easy_setopt(curl, (CURLoption)85, 1L);
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L); // Opțional: creează foldere pe server dacă încarci
    }

    // Autentificare (dacă e furnizată)
    if (!userPwd.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, userPwd.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);

    if (res != CURLE_OK) {
        lastError = curl_easy_strerror(res);
        LOG_ERROR(L"CURL Error (" + std::to_wstring(res) + L"): " + std::wstring(lastError.begin(), lastError.end()));
        return false;
    }

    LOG_SUCCESS(L"Download reușit: " + std::wstring(url.begin(), url.end()));
    return true;
}