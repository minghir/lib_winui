#include "vCanvas.hpp"
#include "ConsoleManager.hpp"
#include "FontManager.hpp"
#include "../stringUtils.hpp"

ATOM vCanvas::s_canvasClassAtom = 0;

vCanvas::vCanvas(HINSTANCE hInst, const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher)
    : vControl(hInst, id, x, y, width, height, dispatcher),
    m_backgroundColor(RGB(255, 255, 255)) // Default alb
{
    m_ControlType = ControlType::Unknown; // Poți adăuga Canvas în enum-ul tău dacă vrei

    if (s_canvasClassAtom == 0) {
        s_canvasClassAtom = registerCanvasClass(hInst);
    }
}

vCanvas::~vCanvas() {
    // Obiectele GDI create local în onPaint sunt curățate acolo
}

ATOM vCanvas::registerCanvasClass(HINSTANCE hInstance) {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = vControl::StaticWndProc; // Folosim procedura ta statică de bază
    wc.hInstance = hInstance;
    wc.lpszClassName = L"VCanvasClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    // Nu setăm hbrBackground pentru că facem Double Buffering manual

    return RegisterClassEx(&wc);
}

void vCanvas::create(HWND parent) {
    if (!parent) return;

    // Scalăm dimensiunile conform DPI-ului părintelui (folosind logica din vControl)
    UINT parentDpi = GetDpiForWindow(parent);
    scale(parentDpi);

    m_handle = CreateWindowEx(
        0,
        L"VCanvasClass",
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        getX(), getY(), getWidth(), getHeight(),
        parent,
        (HMENU)(uintptr_t)getWin32Id(),
        m_hInstance,
        this // Transmitem 'this' pentru GWLP_USERDATA în StaticWndProc
    );

    if (m_handle) {
        LOG_INFO(L"[vCanvas::create] Canvas creat cu succes: " + str_to_wstr(m_id));
    }
}

void vCanvas::setBackgroundColor(COLORREF color) {
    m_backgroundColor = color;
    update(); // Folosește metoda ta din vControl care apelează InvalidateRect
}

LRESULT vCanvas::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 1; // Blocăm ștergerea fundalului pentru a elimina flicker-ul

    case WM_PAINT:
        onPaint(hwnd);
        return 0;

    case WM_LBUTTONDOWN:
        // Exemplu: Poți propaga evenimentul de click prin dispatcher-ul tău
        onClick();
        return 0;
    case WM_SETCURSOR:
        // Dacă vrei cursorul standard de săgeată:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return 0; // Spunem sistemului că am gestionat noi cursorul
    }

    // Lăsăm vControl să gestioneze restul (resize, move, etc.)
    return vControl::handleMessage(hwnd, msg, wParam, lParam);
}

void vCanvas::onPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    // Double buffering setup...
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // --- INTEGRARE FONT MANAGER ---
    // Obținem fontul scalat folosind datele stocate în vControl (m_fontName, m_baseFontSize)
    HFONT hFont = FontManager::getInstance().getScaledFont(m_fontName, m_baseFontSize, m_currentDpi);
    HGDIOBJ oldFont = SelectObject(memDC, hFont);

    // Curățăm fundalul
    HBRUSH hBrush = CreateSolidBrush(m_backgroundColor);
    FillRect(memDC, &rc, hBrush);
    DeleteObject(hBrush);

    if (m_onDraw) {
        SetBkMode(memDC, TRANSPARENT);
        m_onDraw(memDC, width, height);
    }

    // Copiem pe ecran
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // Cleanup local
    SelectObject(memDC, oldFont); // Nu ștergem hFont aici! FontManager se ocupă de asta.
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}