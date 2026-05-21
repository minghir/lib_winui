#include "vTabControl.hpp"
#include "vGrid.hpp"
#include "..\stringUtils.hpp"
#include <windowsx.h>

vTabControl::vTabControl(
    HINSTANCE hInstance,
    const std::string& id,
    int x, int y, int width, int height, EventDispatcher& dispatcher
) : vContainer(hInstance, id, x, y, width, height, dispatcher)
{
   // ConsoleManager::getInstance().log(L"[vTabControl::Constructor] Called for ID: " + str_to_wstr(id));

    // Inițializare Common Controls (necesar pentru WC_TABCONTROL)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);
}

void vTabControl::create(HWND parent) {
  //  LOG_DEBUG(L"[vTabControl::create] Creating for ID: " + str_to_wstr(m_id));

    if (!parent) return;

    // Aplicăm scalarea DPI înainte de creare (ca în vPanel)
    UINT parentDpi = GetDpiForWindow(parent);
    scale(parentDpi);
    /*
    m_handle = CreateWindowEx(
        WS_EX_CONTROLPARENT, // WS_EX_CONTROLPARENT dacă ai controale care au nevoie de navigare prin tab
        WC_TABCONTROL,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        getX(), getY(), getWidth(), getHeight(),
        parent,
        (HMENU)(uintptr_t)getWin32Id(),
        m_hInstance,
        this
    );
    */
    /*
    m_handle = CreateWindowEx(
        WS_EX_CONTROLPARENT,
        WC_TABCONTROL,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_BUTTONS ,
        getX(), getY(), getWidth(), getHeight(),
        parent,
        (HMENU)(uintptr_t)getWin32Id(),
        m_hInstance,
        this
    );
    */

    m_handle = CreateWindowEx(
        WS_EX_CONTROLPARENT, // Permite navigarea prin tab în interiorul paginilor
        WC_TABCONTROL,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Elimină TCS_BUTTONS dacă vrei aspect de tab-uri lipite
        getX(), getY(), getWidth(), getHeight(),
        parent,
        (HMENU)(uintptr_t)getWin32Id(),
        m_hInstance,
        this
    );

    if (!m_handle) {
        LOG_ERROR(L"[ERROR] vTabControl creation failed. Error: " + std::to_wstring(GetLastError()));
    }
    else {
        // Inițializăm fontul și rect-ul original (ca în vPanel)
        scaleFont(getCurrentDpi());
        GetClientRect(m_handle, &m_originalClientRect);

      //  LOG_SUCCESS(L"[vTabControl::create] Success. HWND: " + std::to_wstring(reinterpret_cast<uintptr_t>(m_handle)));
    }
}
/*
void vTabControl::addTabPage(const std::wstring& title, std::unique_ptr<vPanel> page) {
    if (!m_handle) return;

    // 1. Inserare tab vizual
    TCITEMW tie = { 0 };
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)title.c_str();
    int index = TabCtrl_GetItemCount(m_handle);
    SendMessageW(m_handle, TCM_INSERTITEMW, index, (LPARAM)&tie);

    vPanel* pPage = page.get();

    // 2. STOCARE: Transferăm proprietatea către vTabControl (vContainer)
    // Astfel, pPage va trăi cât timp trăiește Tab-ul.
    std::string pageId = "tab_page_" + std::to_string(index);
    this->addChild(pageId, std::move(page), m_handle);

    // 3. CREARE HWND: 
    // Părintele panelului trebuie să fie PĂRINTELE TabControl-ului (MainWindow)
    // pentru a putea fi desenat corect deasupra.
    HWND hParent = GetParent(m_handle);
    //pPage->create(hParent);

    // 4. POZIȚIONARE: Ajustăm dimensiunea să nu acopere titlurile
    RECT rc;
    GetWindowRect(m_handle, &rc);

    // Convertim coordonatele de la Screen la Fereastra Părinte (MainWindow)
    // Este mai sigur decât ScreenToClient pentru că ajustează ambele puncte (left, top, right, bottom)
    MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);

    // Ajustăm RECT-ul pentru a lăsa loc butoanelor de tab de sus
    SendMessage(m_handle, TCM_ADJUSTRECT, FALSE, (LPARAM)&rc);

    // Mutăm panelul în zona calculată
    pPage->moveAndResize(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

    // 5. GESTIONARE PAGINI (pentru switchPage)
    m_pages.push_back({ title, pPage });

    // Prima pagină trebuie să fie vizibilă, celelalte ascunse
    if (index == 0) {
        pPage->show();
        pPage->applyLayout();
    }
    else {
        pPage->hide();
    }
}
*/

void vTabControl::addTabPage(const std::wstring& title, std::unique_ptr<vPanel> page) {
    if (!m_handle) return;

    int index = TabCtrl_GetItemCount(m_handle);
    TCITEMW tie = { 0 };
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)title.c_str();
    SendMessageW(m_handle, TCM_INSERTITEMW, index, (LPARAM)&tie);

    vPanel* pPage = page.get();
    std::string pageId = "tab_page_" + std::to_string(index);

    // AICI: Setăm Tab-ul ca părinte HWND (m_handle)
    this->addChild(pageId, std::move(page), m_handle);

    // Calculăm dimensiunea interioară (Display Area) a Tab-ului
    RECT rc;
    GetClientRect(m_handle, &rc); // Luăm (0, 0, width, height) ale tab-ului
    SendMessage(m_handle, TCM_ADJUSTRECT, FALSE, (LPARAM)&rc); // Elimină header-ul cu butoane

    // IMPORTANT: Nu mai folosim MapWindowPoints pentru hParent!
    // Deoarece pPage este acum COPILUL lui m_handle, (rc.left, rc.top) 
    // sunt deja coordonatele corecte în interiorul tab-ului.
    pPage->moveAndResize(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

    m_pages.push_back({ title, pPage });

    if (index == 0) {
        pPage->show();
        pPage->applyLayout();
    }
    else {
        // Ascundere agresivă pentru a evita suprapunerea controalelor cu culori custom
        pPage->hide();
        SetWindowPos(pPage->getHandle(), NULL, -10000, -10000, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
    }
}

/*
void vTabControl::switchPage(int index) {
    if (index < 0 || index >= (int)m_pages.size() || !m_handle) return;

    // 1. Obținem zona de afișare (fără butoane) în coordonate locale
    RECT rc = { 0, 0, m_width, m_height };
    SendMessage(m_handle, TCM_ADJUSTRECT, FALSE, (LPARAM)&rc);

    for (int i = 0; i < (int)m_pages.size(); ++i) {
        vPanel* pPage = m_pages[i].panel;
        if (i == index) {
            // 2. Coordonatele trebuie să fie X/Y-ul Tab-ului + offset-ul calculat de TCM_ADJUSTRECT
            int finalX = getX() + rc.left;
            int finalY = getY() + rc.top;
            int finalW = rc.right - rc.left;
            int finalH = rc.bottom - rc.top;

            pPage->setWidth(finalW);
            pPage->setHeight(finalH);

            // Folosim SWP_NOACTIVATE și ne asigurăm că nu punem HWND_TOP 
            // dacă asta forțează acoperirea header-ului.
            SetWindowPos(pPage->getHandle(), NULL,
                finalX, finalY, finalW, finalH,
                SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

            pPage->applyLayout();
            InvalidateRect(pPage->getHandle(), NULL, TRUE);

            RedrawWindow(pPage->getHandle(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
        }
        else {
            pPage->hide();
        }
    }
}
*/

void vTabControl::switchPage(int index) {
    if (index < 0 || index >= (int)m_pages.size() || !m_handle) return;

    for (int i = 0; i < (int)m_pages.size(); ++i) {
        vPanel* pPage = m_pages[i].panel;
        HWND hPage = pPage->getHandle();
        if (!hPage) continue;

        if (i == index) {
            // Calculăm zona de afișare locală a tab-ului
            RECT rc = { 0, 0, m_width, m_height };
            TabCtrl_AdjustRect(m_handle, FALSE, &rc);
            
            int localX = rc.left;
            int localY = rc.top;
            int localW = rc.right - rc.left;
            int localH = rc.bottom - rc.top;

            // Aduce pagina în față și o poziționează corect
            SetWindowPos(hPage, HWND_TOP, localX, localY, localW, localH, SWP_SHOWWINDOW);
            pPage->show();
            pPage->applyLayout();

            // Forțăm un repaint total pe pagină și copii
            RedrawWindow(hPage, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
        else {
            // SOLUȚIA RADICALĂ: Mutăm pagina la coordonate negative
            // Asta garantează că niciun copil (ca lbl2) nu mai este în zona vizibilă
            SetWindowPos(hPage, NULL, -10000, -10000, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOZORDER);
            pPage->hide();
        }
    }
}

int vTabControl::getSelectedIndex() const {
    return (m_handle) ? TabCtrl_GetCurSel(m_handle) : 0;
}

RECT vTabControl::getDisplayRect() {
    RECT rc = { 0, 0, getWidth(), getHeight() };
    if (m_handle) {
        TabCtrl_AdjustRect(m_handle, FALSE, &rc);
    }
    return rc;
}

vPanel* vTabControl::getCurrentPage() const {
    int index = getSelectedIndex();

    // Verificăm dacă indexul este valid (TabCtrl_GetCurSel returnează -1 dacă nu e nimic selectat)
    if (index >= 0 && index < static_cast<int>(m_pages.size())) {
        return m_pages[index].panel;
    }

    return nullptr;
}

LRESULT vTabControl::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // Tratăm notificările specifice (click pe tab)
    if (msg == WM_NOTIFY) {
        LPNMHDR lpnmhdr = reinterpret_cast<LPNMHDR>(lParam);
        if (lpnmhdr->code == TCN_SELCHANGE) {
           // LOG_DEBUG(L"Click Aicic!!!!");
            switchPage(TabCtrl_GetCurSel(m_handle));

            return 0;
        }
        else {
           // return getCurrentPage()->handleMessage(hwnd, msg, wParam, lParam);
        }
    }

    // Pentru WM_SIZE, procedăm ca în vPanel
    
    if (msg == WM_SIZE) {
        // 1. Updatează dimensiunile interne ale obiectului C++
        m_width = LOWORD(lParam);
        m_height = HIWORD(lParam);

        // 2. Lasă containerul să-și facă logica (scale, etc)
        LRESULT res = vContainer::handleMessage(hwnd, msg, wParam, lParam);

        // 3. REPOZIȚIONEAZĂ PAGINA CURENTĂ!
        // Fără asta, switchPage nu e apelat la resize, deci pg1 rămâne mic.
       
        switchPage(getSelectedIndex());
        return res;
    }
    /*
    if (msg == WM_DRAWITEM) {
        LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
        if (lpdis->CtlType == ODT_TAB) {
            WCHAR szText[256];
            TCITEM tie = { 0 };
            tie.mask = TCIF_TEXT;
            tie.pszText = szText;
            tie.cchTextMax = 256;
            TabCtrl_GetItem(m_handle, lpdis->itemID, &tie);

            HDC hdc = lpdis->hDC;
            RECT rc = lpdis->rcItem;

            // Verificăm dacă este tab-ul selectat
            bool isSelected = (lpdis->itemID == (UINT)TabCtrl_GetCurSel(m_handle));

            // Creăm/Obținem fontul (Bold pentru selectat, Normal pentru restul)
            HFONT hFontOld;
            if (isSelected) {
                // Poți stoca acest font bold ca membru în clasă ca să nu îl creezi de 100 de ori pe secundă
                LOGFONT lf;
                GetObject(GetWindowFont(m_handle), sizeof(lf), &lf);
                lf.lfWeight = FW_BOLD;
                HFONT hFontBold = CreateFontIndirect(&lf);
                hFontOld = (HFONT)SelectObject(hdc, hFontBold);

                SetTextColor(hdc, RGB(0, 0, 0)); // Culoare text selectat
                // Opțional: DeleteObject(hFontBold) după ce termini de desenat
            }
            else {
                hFontOld = (HFONT)SelectObject(hdc, GetWindowFont(m_handle));
                SetTextColor(hdc, RGB(100, 100, 100)); // Culoare text neselectat
            }

            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, hFontOld);
            return TRUE;
        }
    }
    if (msg == WM_MEASUREITEM) {
        LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
        if (lpmis->CtlType == ODT_TAB) {
            // Setează dimensiunea dorită pentru tab-uri
            lpmis->itemWidth = 120;  // Lățimea în pixeli
            lpmis->itemHeight = 25;  // Înălțimea în pixeli
            return TRUE;
        }
    }
    */

    return vContainer::handleMessage(hwnd, msg, wParam, lParam);
}

void vTabControl::moveAndResize(int x, int y, int width, int height) {
    // 1. Apelăm baza ca să mișcăm efectiv obiectul Tab vizual
    vControl::moveAndResize(x, y, width, height);

    // 2. IMPORTANT: Actualizăm rect-ul original dacă e prima dată, 
    // sau pur și simplu ne asigurăm că m_width/m_height sunt la zi
    m_width = width;
    m_height = height;

    // 3. Forțăm repoziționarea paginii curente
    // Asta va declanșa switchPage, care la rândul lui apelează applyLayout pe pagină
    switchPage(getSelectedIndex());
}

