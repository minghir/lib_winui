#ifndef PDF_DOCUMENT_HPP
#define PDF_DOCUMENT_HPP
#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <windows.h>

extern "C" {

#include <mupdf/fitz.h>
#include <mupdf/pdf.h>

}

class PdfDocument {
private:
    fz_context* m_ctx = nullptr;
    fz_document* m_doc = nullptr;
    std::vector<unsigned char> m_memoryBuffer; // Păstrează buffer-ul viu în RAM

public:
    PdfDocument();
    ~PdfDocument();
    // Deschidere din memorie (Vector de bytes - perfect pentru aplicația ta)
    bool openFromMemory(const std::vector<uint8_t>& data, const std::string& name = "memory.pdf");

    void close();;

    // Randarea unei pagini direct într-un fz_pixmap
    //fz_pixmap* renderPageToPixmap(int pageIndex, float resolution);
    fz_pixmap* renderPage(int pageIndex, float resolution);

    fz_context* getContext() const { return m_ctx; }
    bool isOpen() const { return m_doc != nullptr; }

    int getPageCount() const;
    bool openFromFile(const std::wstring& filePath);

    bool print(HWND ownerHwnd, const std::wstring& docTitle = L"Document PDF");
    bool saveToFile(const std::wstring& filePath);
};

#endif