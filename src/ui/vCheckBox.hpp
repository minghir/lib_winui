#ifndef VCHECKBOX_HPP
#define VCHECKBOX_HPP

#pragma once
#include "vButton.hpp"

class vCheckBox : public vButton {
public:
    vCheckBox(HINSTANCE hInstance, const std::string& id, const std::wstring& label, int x, int y, int width, int height, EventDispatcher& dispatcher);

    void create(HWND parent) override;

    // Metode specifice pentru starea CheckBox-ului
    bool isChecked() const;
    void setChecked(bool checked);

    // Suprascriem onClick pentru a gestiona logica de bifare
    void onClick() override;

private:
    bool m_checked = false;
};

#endif