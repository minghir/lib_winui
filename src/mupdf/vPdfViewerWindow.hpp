#ifndef VPDFVIEWERWINDOW_HPP
#define VPDFVIEWERWINDOW_HPP

#pragma once

#include "..\ui\vWindow.hpp"
#include "PdfDocument.hpp"
#include <vector>

class vPdfViewerWindow : public vWindow {
private:
    PdfDocument m_pdfDoc;
    fz_pixmap* m_currentPixmap = nullptr;
    int m_currentPage = 0;
    float m_dpi = 96.0f;
    float m_zoomFactor = 1.0f; // 1.0 = 100%, 1.25 = 125%, etc. 


    bool m_isDragging = false;
    POINT m_lastMousePos = { 0, 0 };
    int m_offsetX = 0;
    int m_offsetY = 0;

    void freeCurrentPixmap();
    void renderCurrentPage();

public:
    vPdfViewerWindow(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher);
    ~vPdfViewerWindow();
    bool setupLoadedDocument();
    bool loadPdfFromMemory(const std::vector<uint8_t>& buffer);
    bool loadPdfFromFile(const std::wstring& filePath);
    // Suprascriem gestionarea mesajelor WinAPI din vContainer / vWindow
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    void nextPage();
    void prevPage();
    void goToPage(int page);

    void zoomIn();
    void zoomOut();
    void setZoom(float factor);
    void resetZoom();

    int getCurrentPage() const { return m_currentPage + 1; }
    int getTotalPages() const { return m_pdfDoc.getPageCount(); }
    float getZoomFactor() const { return m_zoomFactor; }

    bool printCurrentDocument() {
        return m_pdfDoc.print(m_handle, L"Raport ANC PDF");
    }

    bool savePdfAs();

};

#endif // VPDFVIEWERWINDOW_HPP