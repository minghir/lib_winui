#include "vProgressBar.hpp"
#include "ConsoleManager.hpp"
#include <commctrl.h> // Necesar pentru PROGRESS_CLASS și mesajele PBM_*

// Constructor principal
vProgressBar::vProgressBar(HINSTANCE hInstance, const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher)
    : vControl(hInstance, id, x, y, width, height, dispatcher)
{
    m_ControlType = ControlType::ProgressBar;
}

// Crearea efectivă a controlului WinAPI
void vProgressBar::create(HWND parent)
{
    if (!parent) {
        ConsoleManager::getInstance().log(
            L"[ERROR] vProgressBar::create: Părintele HWND este nullptr."
        );
        return;
    }

    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
    if (!hInstance) {
        LOG_ERROR(L"[ERROR] vProgressBar::create: Nu s-a putut obține HINSTANCE.");
        return;
    }

    UINT parentDpi = GetDpiForWindow(parent);
    scale(parentDpi);

    // Stiluri pentru progress bar (poti adăuga PBS_SMOOTH dacă dorești bară continuă)
    DWORD dwStyle = WS_CHILD | WS_VISIBLE;

    m_handle = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        nullptr,
        dwStyle,
        getX(), getY(), getWidth(), getHeight(),
        parent,
        (HMENU)(uintptr_t)getWin32Id(),
        hInstance,
        this
    );

    if (!m_handle) {
        ConsoleManager::getInstance().log(
            L"[ERROR] vProgressBar::create: Eroare la crearea HWND. Cod: "
            + std::to_wstring(GetLastError())
        );
        return;
    }

    // Inițializăm intervalul și valoarea setate anterior
    setRange(m_minVal, m_maxVal);
    setValue(m_currentVal);

    GetClientRect(m_handle, &m_originalClientRect);
}

// Gestionarea mesajelor
LRESULT vProgressBar::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return vControl::handleMessage(hwnd, msg, wParam, lParam);
}

// Setează limitele minime și maxime ale barei de progres
void vProgressBar::setRange(int minVal, int maxVal) {
    m_minVal = minVal;
    m_maxVal = maxVal;
    if (m_handle) {
        SendMessage(m_handle, PBM_SETRANGE32, (WPARAM)minVal, (LPARAM)maxVal);
    }
}

// Setează valoarea curentă a progresului
void vProgressBar::setValue(int value) {
    m_currentVal = value;
    if (m_handle) {
        SendMessage(m_handle, PBM_SETPOS, (WPARAM)value, 0);
    }
}

// Returnează valoarea curentă
int vProgressBar::getValue() const {
    if (m_handle) {
        return (int)SendMessage(m_handle, PBM_GETPOS, 0, 0);
    }
    return m_currentVal;
}

// Avansează progresul cu un pas (delta)
void vProgressBar::step(int delta) {
    m_currentVal += delta;
    if (m_handle) {
        SendMessage(m_handle, PBM_DELTAPOS, (WPARAM)delta, 0);
    }
}