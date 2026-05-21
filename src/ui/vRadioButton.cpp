#include "vRadioButton.hpp"
#include "vContainer.hpp"
#include "../stringUtils.hpp"
#include <windows.h>

vRadioButton::vRadioButton(HINSTANCE hInstance, const std::string& id, const std::wstring& label,
    int x, int y, int width, int height, EventDispatcher& dispatcher)
    : vButton(hInstance, id, label, x, y, width, height, dispatcher)
{
    m_ControlType = ControlType::RadioButton; // Presupunând că ai acest enum
}

void vRadioButton::create(HWND parent) {
    if (!parent) return;

    UINT parentDpi = GetDpiForWindow(parent);
    scale(parentDpi);

    const int buttonId = getWin32Id();
    // Folosim BS_RADIOBUTTON în loc de BS_AUTORADIOBUTTON pentru control manual
    uint32_t style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON;

    m_handle = CreateWindowEx(
        0,
        L"BUTTON",
        m_label.c_str(),
        style,
        m_x, m_y, m_width, m_height,
        parent,
        (HMENU)(INT_PTR)buttonId,
        m_hInstance,
        this
    );

    if (m_handle) {
        scaleFont(getCurrentDpi());
        SendMessage(m_handle, BM_SETCHECK, m_checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool vRadioButton::isChecked() const {
    if (!m_handle) return false;
    return SendMessage(m_handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void vRadioButton::setChecked(bool checked) {
    LOG_DEBUG(L"setChecked apelat pentru " + str_to_wstr(m_id) + L" cu valoarea: " + (checked ? L"TRUE" : L"FALSE"));
    m_checked = checked;
    if (m_handle) {
        SendMessage(m_handle, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
        // Forțează butonul să se redeseneze ca să apară bifa
        InvalidateRect(m_handle, NULL, TRUE);
        UpdateWindow(m_handle);
    }
}

void vRadioButton::onClick() {
    this->setChecked(true);
    LOG_DEBUG(L"sunt aici pentru:" + str_to_wstr(getId()));
    // Luăm containerul care deține acest control
    vContainer* parentContainer = dynamic_cast<vContainer*>(this->getParent());

    if (parentContainer) {
        auto& children = parentContainer->getChildren();
        LOG_DEBUG(L"Caut in containerul " + str_to_wstr(parentContainer->getId()) + L" care are " + std::to_wstring(children.size()) + L" copii");

        for (auto& pair : children) {
            vControl* ctrl = pair.second.get();
            // Verificăm tipul manual dacă dynamic_cast e suspect
            if (ctrl->getType() == ControlType::RadioButton) {
                vRadioButton* rb = static_cast<vRadioButton*>(ctrl);

                if (rb != this && rb->getGroupName() == this->m_groupName) {
                    LOG_DEBUG(L"Găsit coleg de grup: " + str_to_wstr(rb->getId()) + L". Fac UNCHECK.");
                    rb->setChecked(false);
                }
            }
        }
    }
    vButton::onClick();
}