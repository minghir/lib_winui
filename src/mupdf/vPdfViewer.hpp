#ifndef VPDFVIEWER_HPP
#define VPDFVIEWER_HPP

#pragma once

#include "vPdfViewerWindow.hpp"
#include "..\ui\vMenu.hpp"
#include <memory>

class vPdfViewer {
private:
    std::unique_ptr<vPdfViewerWindow> m_pWindow;
    vPdfViewerWindow* m_windowPtr = nullptr;

    std::unique_ptr<vMenu> m_mainMenu;

    // Submeniuri create ca instanțe vMenu
    std::unique_ptr<vMenu> m_fileMenu;
    std::unique_ptr<vMenu> m_viewMenu;
    std::unique_ptr<vMenu> m_navMenu;

    EventDispatcher& m_dispatcher;

    void buildMenu();
    void registerEvents();

public:
    vPdfViewer(HINSTANCE hInstance, EventDispatcher& dispatcher);
    ~vPdfViewer();// = default;

    bool create(HWND parent, const std::wstring& title = L"PDF Viewer");

    bool loadFromMemory(const std::vector<uint8_t>& buffer);
    bool loadFromFile(const std::wstring& filePath);

    vPdfViewerWindow* getWindow() const { return m_pWindow.get(); }

    std::unique_ptr<vPdfViewerWindow> extractWindow();

};

#endif // VPDFVIEWER_HPP