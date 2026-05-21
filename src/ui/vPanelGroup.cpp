#include "vPanelGroup.hpp"

vPanelGroup::vPanelGroup(HINSTANCE hInst, const std::string& id, const std::wstring& title,
    int x, int y, int w, int h, EventDispatcher& disp)
    : vPanel(hInst, id, x, y, w, h, disp), m_title(title) {
    // În constructorul de vPanel, clasa se înregistrează deja cu o procedură proprie
}

LRESULT vPanelGroup::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);

        // 1. DESENĂM FUNDALUL PANELULUI
        FillRect(hdc, &rc, getEffectiveBackgroundBrush());

        // 2. DETERMINĂM CULORILE
        COLORREF frameColor = (m_borderColor != CLR_INVALID) ? m_borderColor : getEffectiveTextColor();
        COLORREF textColor = getEffectiveTextColor();
        COLORREF bgColor = getEffectiveBackgroundColor();

        // 3. PREGĂTIM FONTUL ȘI LOGĂM STAREA LUI
        HFONT hFont = getEffectiveFont();

        // --- LOG DEBUG ---
        LOG_DEBUG(L"[vPanelGroup] Paint ID: " + str_to_wstr(m_id) +
            L" | HFONT Handle: " + std::to_wstring((uintptr_t)hFont) +
            L" | Title: " + m_title);

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        HPEN hPen = CreatePen(PS_SOLID, 1, frameColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

        // 4. DESENĂM RAMA
        int frameTop = 15;
        Rectangle(hdc, rc.left + 1, rc.top + frameTop, rc.right - 1, rc.bottom - 1);

        // 5. CONFIGURARE TEXT
        SetTextColor(hdc, textColor);
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, bgColor);

        // 6. CALCULĂM ȘI DESENĂM TITLUL
        RECT textRect;
        textRect.left = m_titleOffset;
        textRect.top = 0;
        textRect.right = rc.right - 10;
        textRect.bottom = 100; // Mai mare pentru orice eventualitate

        // Masuram
        DrawTextW(hdc, m_title.c_str(), -1, &textRect, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);

        int textHeight = textRect.bottom - textRect.top;
        int textWidth = textRect.right - textRect.left;

        // --- LOG DIMENSIUNI ---
        LOG_DEBUG(L"[vPanelGroup] Measured Text Height: " + std::to_wstring(textHeight) +
            L" px | DPI: " + std::to_wstring(GetDpiForWindow(hwnd)));

        // Padding si pozitionare
        int padding = 6;
        textRect.left = m_titleOffset;
        textRect.right = textRect.left + textWidth + (padding * 2);

        textRect.top = frameTop - (textHeight / 2);
        textRect.bottom = textRect.top + textHeight;

        // Desenarea efectivă
        DrawTextW(hdc, m_title.c_str(), -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

        // 7. CURĂȚENIE
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldFont);

        DeleteObject(hPen);

        EndPaint(hwnd, &ps);
        return 0;
    }

    return vPanel::handleMessage(hwnd, msg, wParam, lParam);
}

void vPanelGroup::setTitle(const std::wstring& title) {
    
    
    m_title = title;

    // Verificăm dacă obiectul are deja un handle Win32 creat
    if (m_handle) {
        // 1. Opțional: Actualizăm și textul ferestrei la nivel de OS 
        // (util pentru accesibilitate sau unelte de debugging)
        SetWindowTextW(m_handle, title.c_str());

        // 2. Forțăm o redesenare completă a controlului.
        // NULL înseamnă toată zona controlului.
        // TRUE înseamnă că va trimite și un mesaj WM_ERASEBKGND înainte de WM_PAINT.
        InvalidateRect(m_handle, NULL, TRUE);

        // 3. (Opțional) Dacă vrei ca desenarea să fie instantanee, fără a aștepta 
        // coada de mesaje a Windows-ului:
        UpdateWindow(m_handle);
    }
}

void vPanelGroup::setBorderColor(COLORREF col) {
    m_borderColor = col;

    // Dacă fereastra a fost deja creată, cerem redesenarea
    if (m_handle) {
        // Redesenăm doar bordura și titlul
        // TRUE indică faptul că fundalul trebuie șters (WM_ERASEBKGND)
        InvalidateRect(m_handle, NULL, TRUE);

        // Opțional: forțăm procesarea imediată a mesajului WM_PAINT
        UpdateWindow(m_handle);
    }
}