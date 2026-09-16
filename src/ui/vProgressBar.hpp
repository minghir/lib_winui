#ifndef VPROGRESSBAR_HPP
#define VPROGRESSBAR_HPP

#pragma once

#include "vControl.hpp"
#include <string>

class vProgressBar : public vControl {
public:
    // Constructor principal
    explicit vProgressBar(HINSTANCE hInstance, const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher);

    // Constructor simplificat pentru XML / Factory
    explicit vProgressBar(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher)
        : vProgressBar(hInstance, id, 0, 0, 200, 25, dispatcher)
    {
    }

    // Destructor
    virtual ~vProgressBar() = default;

    // Suprascriere metode din vControl
    void create(HWND parent) override;
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    // Metode specifice pentru manipularea progresului
    void setRange(int minVal, int maxVal);
    void setValue(int value);
    int getValue() const;
    void step(int delta = 1);

    void scale(int newDpi) override {
        vControl::scale(newDpi);
        // Dacă dorești scalare suplimentară specifică pentru bară
    }

private:
    int m_minVal = 0;
    int m_maxVal = 100;
    int m_currentVal = 0;
};

#endif // VPROGRESSBAR_HPP