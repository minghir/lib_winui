#ifndef VSAVEDIALOG_HPP
#define VSAVEDIALOG_HPP

#include <windows.h>
#include <commdlg.h>
#include <string>

/**
 * @brief Utilitar pentru dialogul standard Windows "Save As".
 * Nu moștenește vControl deoarece este un dialog de sistem efemer (modal).
 */
class vSaveFileDialog {
public:
    vSaveFileDialog(const std::wstring& title = L"Save File")
        : m_title(title), m_filter(L"All Files\0*.*\0"), m_defExt(L"") {
    }

    // Setează filtrele. Format: L"Nume\0*.ext\0AltNume\0*.alt\0"
    void setFilter(const std::wstring& filter) { m_filter = filter; }

    // Setează extensia implicită (ex: L"csv")
    void setDefaultExtension(const std::wstring& ext) { m_defExt = ext; }

    /**
     * @brief Deschide dialogul.
     * @param parent Handle-ul ferestrei care va fi "blocată" de acest dialog.
     * @return true dacă utilizatorul a apăsat OK, false la Cancel.
     */
    bool show(HWND parent = nullptr) {
        wchar_t szFile[MAX_PATH] = { 0 };
        OPENFILENAMEW ofn = { 0 };

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = parent;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = m_filter.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = m_title.c_str();
        ofn.lpstrDefExt = m_defExt.empty() ? nullptr : m_defExt.c_str();

        // OFN_OVERWRITEPROMPT: întreabă utilizatorul înainte de a suprascrie
        // OFN_PATHMUSTEXIST: asigură că folderul există
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

        if (GetSaveFileNameW(&ofn)) {
            m_resultPath = szFile;
            return true;
        }
        return false;
    }

    std::wstring getFilePath() const { return m_resultPath; }

private:
    std::wstring m_title;
    std::wstring m_filter;
    std::wstring m_defExt;
    std::wstring m_resultPath;
};

#endif