#pragma once

#include "vControl.hpp"
#include <string>

class vStatusBar : public vControl {
public:
    // Constructor
    vStatusBar(const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher);

    // Creates the WinAPI status bar window.
    void create(HWND parentHwnd) override;

    // Sets the text for a specific part of the status bar.
    void setPartText(int partIndex, const std::wstring& text);

    // Sets the number of parts and their widths.
    void setParts(const std::vector<int>& widths);

    void resize();

    void scale(int newDpi) override {
        // Apelăm implementarea din vContainer, care conține bucla de propagare la copii
       
        vControl::scale(newDpi);
        this->scaleFont(newDpi);
        
    }
private:
    std::vector<int> m_parts;
};