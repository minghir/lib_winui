#ifndef VPANELGROUP_HPP
#define VPANELGROUP_HPP

#include "vPanel.hpp"

class vPanelGroup : public vPanel {
protected:
    std::wstring m_title;
    int m_titleOffset = 10; // Unde începe textul pe orizontală
    COLORREF m_borderColor = CLR_INVALID; // Valoare default
public:
    vPanelGroup(HINSTANCE hInst, const std::string& id, const std::wstring& title,
        int x, int y, int w, int h, EventDispatcher& disp);

    // Suprascriem handleMessage pentru desenare custom
    virtual LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    // Putem adăuga o metodă pentru a schimba titlul din mers
    void setTitle(const std::wstring& title);
    void setBorderColor(COLORREF col);
};

#endif