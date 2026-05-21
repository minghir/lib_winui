#include "vCheckBox.hpp"
#include "../stringUtils.hpp"
#include <windows.h>

vCheckBox::vCheckBox(HINSTANCE hInstance, const std::string& id, const std::wstring& label, int x, int y, int width, int height, EventDispatcher& dispatcher)
    : vButton(hInstance, id, label, x, y, width, height, dispatcher)
{
}

void vCheckBox::create(HWND parent) {
    if (!parent) return;

    UINT parentDpi = GetDpiForWindow(parent);
    scale(parentDpi);

    const int controlId = getWin32Id();

    // Folosim clasa "BUTTON" dar cu stilul BS_AUTOCHECKBOX
    // BS_AUTOCHECKBOX face ca Windows să schimbe bifa automat când dai click
    m_handle = CreateWindowEx(
        0,
        L"BUTTON",
        m_label.c_str(),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        m_x, m_y, m_width, m_height,
        parent,
        (HMENU)(INT_PTR)controlId,
        m_hInstance,
        this
    );

    if (m_handle) {
        scaleFont(getCurrentDpi());
        // Setăm starea vizuală inițială în HWND
        SendMessage(m_handle, BM_SETCHECK, m_checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool vCheckBox::isChecked() const {
    if (m_handle) {
        return SendMessage(m_handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }
    return m_checked;
}

void vCheckBox::setChecked(bool checked) {
    m_checked = checked;
    if (m_handle) {
        SendMessage(m_handle, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

void vCheckBox::onClick() {
    // 1. Actualizăm starea internă (bifa)
    m_checked = isChecked();

    // 2. Declanșăm evenimentul specific de schimbare
    // Acesta va face ca pCheck->on("change", ...) să funcționeze
    m_dispatcher.dispatch("change", m_id);

    // 3. Apelăm implementarea de bază din vButton
    // Aceasta va declanșa "click", ceea ce ai testat tu adineauri și a mers
    vButton::onClick();
}