#include "vStatusBar.hpp"
#include <commctrl.h> // For STATUSCLASSNAME
#include "ConsoleManager.hpp"
#include "../stringUtils.hpp"

// Constructor
vStatusBar::vStatusBar(const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher)
    : vControl(nullptr, id, x, y, width, height, dispatcher) {
    //
    m_ControlType = ControlType::StatusBar;
}

void vStatusBar::create(HWND parentHwnd) {

    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parentHwnd, GWLP_HINSTANCE);
    if (!hInstance) {
        ConsoleManager::getInstance().log(L"[ERROR] vButton::create: Nu s-a putut obține HINSTANCE de la părintele HWND: " + std::to_wstring(reinterpret_cast<uintptr_t>(parentHwnd)));
        return;
    }

    UINT parentDpi = GetDpiForWindow(parentHwnd);
    // Apelează metoda de scalare cu DPI-ul obținut
    scale(parentDpi);
/*
    m_handle = CreateWindowExW(
        0,                                   // No extended styles
        STATUSCLASSNAMEW,                    // The status bar class name
        NULL,                                // No default text
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, // Child, visible, with a size grip
        0, 0, 0, 0,                          // Size and position set by parent
        parentHwnd,                          // Parent window
        (HMENU)0,                            // No menu
        hInstance,                          // Instance handle
        NULL);                               // No creation parameters
*/

    m_handle = CreateWindowExW(
        0,
        STATUSCLASSNAMEW,
        NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, // Folosește o dimensiune inițială non-zero
        parentHwnd,
        (HMENU)0,
        hInstance,
        NULL);

    if (!m_handle) {
        ConsoleManager::getInstance().log(L"[ERROR] Failed to create status bar.");
        return ;
    }

   // ConsoleManager::getInstance().log(L"[vStatusBar] Status bar created with ID: " + str_to_wstr(m_id));
    return ;
}
/*
void vStatusBar::setParts(const std::vector<int>& widths) {
    m_parts = widths;
    std::vector<int> parts;
    int totalWidth = 0;
    for (int w : widths) {
        totalWidth += w;
        parts.push_back(totalWidth);
    }
    // Set the parts for the status bar.
    // The -1 at the end indicates the last part is the rest of the bar.
    SendMessage(m_handle, SB_SETPARTS, parts.size(), (LPARAM)parts.data());
}
*/

void vStatusBar::setParts(const std::vector<int>& widths) {
    if (!m_handle) return;

    // Obține DPI-ul curent (presupunând că ai acces la vControl::getCurrentDpi())
    int dpi = getCurrentDpi();
    if (dpi == 0) dpi = 96;

    m_parts = widths;
    std::vector<int> parts;
    int totalWidth = 0;

    for (size_t i = 0; i < widths.size(); ++i) {
        if (widths[i] == -1) {
            parts.push_back(-1);
        }
        else {
            // Scalează fiecare lățime conform DPI
            totalWidth += MulDiv(widths[i], dpi, 96);
            parts.push_back(totalWidth);
        }
    }
    SendMessage(m_handle, SB_SETPARTS, parts.size(), (LPARAM)parts.data());
}

void vStatusBar::setPartText(int partIndex, const std::wstring& text) {
    SendMessage(m_handle, SB_SETTEXT, partIndex, (LPARAM)text.c_str());
}
/*
void vStatusBar::resize() {
    ConsoleManager::getInstance().log(L"ERROR [vStatusBar::resize] incerc resize status bar.");

    //int newWidth = LOWORD(lParam);
    //int newHeight = HIWORD(lParam);

    SendMessage(m_handle, WM_SIZE, 0,0);
  

    // Get the updated client rectangle after the status bar has been sized
    RECT clientRect;
    GetClientRect(GetParent(m_handle), &clientRect);
    int statusBarHeight = 0;
    RECT statusBarRect;
    GetWindowRect(m_handle, &statusBarRect);
    statusBarHeight = statusBarRect.bottom - statusBarRect.top;

}
*/

void vStatusBar::resize() {
    if (!m_handle) return;

    // 1. Spune-i barei să se reașeze (WinAPI nativ)
    SendMessage(m_handle, WM_SIZE, 0, 0);

    // 2. Opțional: Dacă vrei să forțezi redesenarea părților
    // (Uneori părțile trebuie setate din nou la resize dacă nu sunt cu -1)
    if (!m_parts.empty()) {
        setParts(m_parts);
    }
}
