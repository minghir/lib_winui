#ifndef VDATEPICKER_HPP
#define VDATEPICKER_HPP

#include "vControl.hpp"
#include <commctrl.h> // Necesar pentru DATETIMEPICK_CLASS

class vDatePicker : public vControl {
    SYSTEMTIME m_lastDate = { 0 };
public:
    vDatePicker(HINSTANCE hInstance, const std::string& id, int x, int y, int width, int height, EventDispatcher& disp);

    void create(HWND parent) override;

    // Returnează data selectată sub formă de SYSTEMTIME
    SYSTEMTIME getDate() const;
    std::wstring getDataAs(std::wstring format) const;

    void setDate(const SYSTEMTIME& st);
    void setFormat(const std::wstring& format);

    // Helper pentru a scoate data ca string (YYYY-MM-DD)
    std::wstring getDateString() const;

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void scale(int newDpi) override {
        vControl::scale(newDpi);   // Actualizează DPI
        this->scaleFont(newDpi);   // Setează fontul standard (WM_SETFONT pe m_handle)
    }

    void setText(const std::wstring& isoDate);
};

#endif