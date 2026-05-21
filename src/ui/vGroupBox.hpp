#ifndef VGROUPBOX_HPP
#define VGROUPBOX_HPP

#pragma once
#include "vContainer.hpp"


class vGroupBox : public vContainer {
    HWND m_titleHandle = nullptr;
    HFONT m_hFont = nullptr;


    WNDPROC m_originalWndProc = nullptr;

    bool m_showBorder = true;

public:
    explicit vGroupBox(
        HINSTANCE hInstance,
        const std::string& id,
        const std::wstring& title,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher
    );

    virtual ~vGroupBox() = default;

    void create(HWND parent) override;


    void setTitle(const std::wstring& title) {
        m_title = title;
        if (m_handle) {
            // Trimitem textul și către Win32 (pentru orice eventualitate)
            SetWindowTextW(m_handle, title.c_str());
            // Forțăm redesenarea imediată
            InvalidateRect(m_handle, NULL, TRUE);
        }
    }

    void scale(int newDpi) override {
        vContainer::scale(newDpi);
        this->scaleFont(newDpi);
    }

    // Suprascriem pentru a ne asigura că layout-ul copiilor 
    // ține cont de marginea de sus unde e textul (titlul)
    void applyLayout() override;

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    void passkids() {
        for (auto& [id, child] : getChildren()) {
            child->setParent(this->getParent());
        }
    }

    void setShowBorder(bool show) { m_showBorder = show; }
    bool isBorderVisible() const { return m_showBorder; }

private:
    std::wstring m_title;
};

#endif