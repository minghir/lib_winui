#ifndef VCANVAS_HPP
#define VCANVAS_HPP

#pragma once

#include "vControl.hpp"
#include <functional>

class vCanvas : public vControl {
public:
    explicit vCanvas(HINSTANCE hInst, const std::string& id,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher);

    virtual ~vCanvas();

    // Implementăm create pentru a înregistra clasa și a crea HWND-ul
    void create(HWND parent) override;

    // Suprascriem handleMessage pentru desenare
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    // API pentru exterior
    void setBackgroundColor(COLORREF color);

    // Callback-ul de desenare primește HDC-ul, lățimea și înălțimea curentă (scalate)
    void setOnDraw(std::function<void(HDC, int, int)> callback) { m_onDraw = callback; }

protected:
    void onPaint(HWND hwnd);

private:
    COLORREF m_backgroundColor;
    std::function<void(HDC, int, int)> m_onDraw;

    static ATOM registerCanvasClass(HINSTANCE hInstance);
    static ATOM s_canvasClassAtom;
};

#endif