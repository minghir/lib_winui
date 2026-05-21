#ifndef VRADIOBUTTON_HPP
#define VRADIOBUTTON_HPP

#pragma once
#include "vButton.hpp" // Moștenim din vButton pentru a refolosi logica de label și onClick

class vRadioButton : public vButton {
    std::string m_groupName;
    bool m_checked = false;
public:
    vRadioButton(HINSTANCE hInstance, const std::string& id, const std::wstring& label,
        int x, int y, int width, int height, EventDispatcher& dispatcher);

    virtual ~vRadioButton() = default;

    void setGroupName(const std::string& name) { m_groupName = name; }
    std::string getGroupName() const { return m_groupName; }

    void create(HWND parent) override;

    // Metode specifice pentru starea butonului
    bool isChecked() const;
    void setChecked(bool check);

    // Suprascriem onClick pentru a gestiona debifarea fraților
    void onClick() override;
};

#endif