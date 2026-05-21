#include "vDatePicker.hpp"
#include "ConsoleManager.hpp"

vDatePicker::vDatePicker(HINSTANCE hInstance, const std::string& id, int x, int y, int width, int height, EventDispatcher& disp)
    : vControl(hInstance, id, x, y, width, height, disp) {
    m_ControlType = ControlType::DatePicker; // Sau adaugă DatePicker în enum-ul tău
}

void vDatePicker::create(HWND parent) {
    if (!parent) return;

    // Asigură-te că biblioteca common controls e încărcată
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_DATE_CLASSES;
    InitCommonControlsEx(&icex);

    m_handle = CreateWindowEx(
        0,
        DATETIMEPICK_CLASS,
        L"DateTime",
        WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP | DTS_SHORTDATEFORMAT,
        m_x, m_y, m_width, m_height,
        parent,
        (HMENU)(INT_PTR)getWin32Id(),
        m_hInstance,
        this
    );

    if (m_handle) {
        SetWindowLongPtr(m_handle, GWLP_USERDATA, (LONG_PTR)this);
        // --- FORȚARE FORMAT: ZI.LUNĂ.AN ---
        // "dd.MM.yyyy" va afișa de exemplu: 24.03.2026
        // Dacă vrei cu slash, folosește "dd/MM/yyyy"
        const wchar_t* customFormat = L"dd.MM.yyyy";
        SendMessage(m_handle, DTM_SETFORMATW, 0, (LPARAM)customFormat);
        scaleFont(GetDpiForWindow(m_handle));
    }
}

SYSTEMTIME vDatePicker::getDate() const {
    SYSTEMTIME st;
    SendMessage(m_handle, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
    return st;
}

std::wstring vDatePicker::getDateString() const {
    SYSTEMTIME st = getDate();
    wchar_t buf[128];
    swprintf_s(buf, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    return buf;
}

LRESULT vDatePicker::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NOTIFY) {
        LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);
        if (nmhdr->code == DTN_DATETIMECHANGE) {
            LPNMDATETIMECHANGE lpChange = reinterpret_cast<LPNMDATETIMECHANGE>(lParam);

            // Verificăm dacă data este identică cu cea anterioară
            if (memcmp(&lpChange->st, &m_lastDate, sizeof(SYSTEMTIME)) == 0) {
                return 0; // Ignorăm duplicatul
            }

            m_lastDate = lpChange->st; // Actualizăm "cache-ul"
            getEventDispatcher().dispatch("dateChange", getId());
            return 0;
        }
    }
    // FOARTE IMPORTANT: Nu apela vContainer::handleMessage aici dacă vDatePicker 
    // nu are copii, folosește vControl::handleMessage.
    return vControl::handleMessage(hwnd, msg, wParam, lParam);
}

void vDatePicker::setDate(const SYSTEMTIME& st) {
    if (m_handle) {
        // Trimitem mesajul DTM_SETSYSTEMTIME către controlul nativ de Windows
        // GDT_VALID indică faptul că structura SYSTEMTIME conține o dată validă
        if (SendMessage(m_handle, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st)) {
            m_lastDate = st; // Actualizăm și cache-ul intern pentru consistență
        }
    }
}


std::wstring vDatePicker::getDataAs(std::wstring format) const {
    SYSTEMTIME st = getDate();
    std::wstring result = format;

    // Pregătim valorile cu zero-padding
    auto to_ws = [](int val, int width) {
        std::wstringstream ss;
        ss << std::setw(width) << std::setfill(L'0') << val;
        return ss.str();
    };

    std::wstring yyyy = to_ws(st.wYear, 4);
    std::wstring yy = to_ws(st.wYear % 100, 2);
    std::wstring mm = to_ws(st.wMonth, 2);
    std::wstring dd = to_ws(st.wDay, 2);

    // Funcție helper pentru înlocuire globală în string
    auto replaceAll = [&](std::wstring& str, const std::wstring& from, const std::wstring& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    // Ordinea contează: yyyy înainte de yy pentru a nu înlocui parțial
    replaceAll(result, L"yyyy", yyyy);
    replaceAll(result, L"yy", yy);
    replaceAll(result, L"mm", mm);
    replaceAll(result, L"dd", dd);

    return result;
}

void vDatePicker::setFormat(const std::wstring& format) {
    if (m_handle) {
        SendMessage(m_handle, DTM_SETFORMATW, 0, (LPARAM)format.c_str());
    }
}
/*
void vDatePicker::setText(const std::wstring& isoDate) {
    SYSTEMTIME st = { 0 };
    int y, m, d;
    if (swscanf_s(isoDate.c_str(), L"%d-%d-%d", &y, &m, &d) == 3) {
        st.wYear = (WORD)y; st.wMonth = (WORD)m; st.wDay = (WORD)d;
        this->setDate(st);
    }
}
*/

void vDatePicker::setText(const std::wstring& isoDate) {
    if (isoDate.empty()) return;

    SYSTEMTIME st = { 0 };
    int year = 0, month = 0, day = 0;

    // Masca %d-%d-%d va citi numerele și se va opri când întâlnește 
    // un caracter care nu se potrivește (cum e spațiul dintre dată și timp)
    if (swscanf_s(isoDate.c_str(), L"%d-%d-%d", &year, &month, &day) >= 3) {
        st.wYear = (WORD)year;
        st.wMonth = (WORD)month;
        st.wDay = (WORD)day;

        this->setDate(st);
    }
    else {
        LOG_ERROR(L"Eroare parsare dată: " + isoDate);
    }
}