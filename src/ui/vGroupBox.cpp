
#include "vGroupBox.hpp"

vGroupBox::vGroupBox(HINSTANCE hInstance, const std::string& id, const std::wstring& title,
    int x, int y, int width, int height, EventDispatcher& dispatcher)
    : vContainer(hInstance, id, x, y, width, height, dispatcher), m_title(title) {
}


void vGroupBox::create(HWND parent) {
    
    
    m_handle = CreateWindowExW(
        WS_EX_CONTROLPARENT,
        L"BUTTON",
        m_title.c_str(),
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_CLIPSIBLINGS,
        m_x, m_y, m_width, m_height,
        parent,
        (HMENU)(uintptr_t)getWin32Id(),
        m_hInstance,
        this
    );

    if (m_handle) {
        // 1. Legăm handle-ul de obiectul curent (esențial pentru StaticWndProc)
        SetWindowLongPtr(m_handle, GWLP_USERDATA, (LONG_PTR)this);

        // 2. Facem Subclassing: instalăm StaticWndProc și salvăm procedura veche
        m_originalWndProc = (WNDPROC)SetWindowLongPtr(m_handle, GWLP_WNDPROC, (LONG_PTR)vControl::StaticWndProc);
        // Setăm fontul standard de sistem
        this->scaleFont(GetDpiForWindow(m_handle));

        // Creăm toți copiii adăugați anterior cu addChild
        // Le dăm m_handle (GroupBox-ul) ca părinte pentru a fi în interiorul ramei
        for (auto& [id, child] : getChildren()) {
           // child->create(GetParent(m_handle));
            child->create(m_handle);
            SetWindowPos(child->getHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        // Aplicăm strategia de așezare (VerticalStack, Anchor, etc.)
        applyLayout();
    }
}



/*
LRESULT vGroupBox::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
 
    if (msg == WM_ERASEBKGND) {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Luăm culoarea de fundal de la părinte (Panel-ul paginii)
        // Presupunând că vContainer sau clasa ta de bază are stocată o perie (Brush)
        HBRUSH hbr = (HBRUSH)SendMessage(GetParent(hwnd), WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)GetParent(hwnd));
        if (!hbr) hbr = GetSysColorBrush(COLOR_3DFACE);

        FillRect(hdc, &rect, hbr);
        return 1; // Am șters fundalul, nu mai lăsa sistemul să o facă transparent
    }
   
    // Gestionarea culorilor pentru copiii din interior (Radio, Checkbox)
    if (msg == WM_CTLCOLORSTATIC || msg == WM_CTLCOLORBTN) {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        // Cerem peria de fundal de la bunicul (Panel-ul paginii) dacă GroupBox e părinte
        return SendMessage(GetParent(hwnd), msg, wParam, lParam);
    }
 
    ///////////////////////////////////////////////////////
    // 1. Notificările (Click-uri) merg la părinte (Panel)
    if (msg == WM_COMMAND || msg == WM_NOTIFY) {
        return SendMessage(GetParent(hwnd), msg, wParam, lParam);
    }

    // 2. Mesajele specifice layout-ului (ex: WM_SIZE)
    if (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED) {
        vContainer::handleMessage(hwnd, msg, wParam, lParam);
        // Nu returnăm aici, lăsăm și CallWindowProc să știe de resize
    }

    // 3. LOGICA DE DESENARE: Pentru mesaje vizuale, folosim DOAR procedura originală
    // Nu apelăm vContainer aici pentru că acesta s-ar putea să încerce să deseneze fundalul peste ramă
    if (msg == WM_PAINT || msg == WM_ERASEBKGND || msg == WM_NCPAINT || msg == WM_CTLCOLORSTATIC) {
        if (m_originalWndProc) {
            return CallWindowProc(m_originalWndProc, hwnd, msg, wParam, lParam);
        }
    }

    // 4. Pentru orice altceva, lăsăm ierarhia normală și apoi originalul
    vContainer::handleMessage(hwnd, msg, wParam, lParam);
    if (m_originalWndProc) {
        return CallWindowProc(m_originalWndProc, hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
*/
/*
LRESULT vGroupBox::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // 1. GESTIONARE FUNDAL (Transparență/Culoare Părinte)
    
    if (msg == WM_ERASEBKGND) {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        // Cerem peria de la părinte pentru consistență
        HBRUSH hbr = (HBRUSH)SendMessage(GetParent(hwnd), WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)GetParent(hwnd));
        if (!hbr) hbr = GetSysColorBrush(COLOR_3DFACE);
        FillRect(hdc, &rect, hbr);
        return 1;
    }
    
    // 2. LOGICA DE ASCUNDERE CHENAR (Dacă m_showBorder e false)
    if (msg == WM_PAINT && !m_showBorder) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Dacă nu vrem chenar, dar avem titlu, desenăm doar textul
        if (!m_title.empty()) {
            SelectObject(hdc, m_hFont);
            SetBkMode(hdc, TRANSPARENT);
            // Calculăm unde să punem textul (de obicei stânga sus)
            TextOutW(hdc, 10, 0, m_title.c_str(), (int)m_title.length());
        }

        EndPaint(hwnd, &ps);
        return 0; // Oprim execuția aici, nu mai chemăm procedura originală (care ar desena rama)
    }

    // 3. CULORI PENTRU CONTROALELE COPIL (Radio, Checkbox)
    
    if (msg == WM_CTLCOLORSTATIC || msg == WM_CTLCOLORBTN) {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return SendMessage(GetParent(hwnd), msg, wParam, lParam);
    }
    
    // 4. NOTIFICĂRI (Forward către Panel/Fereastră)
    if (msg == WM_COMMAND || msg == WM_NOTIFY) {
        return SendMessage(GetParent(hwnd), msg, wParam, lParam);
    }

    // 5. LAYOUT & RESIZE
    if (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED) {
        vContainer::handleMessage(hwnd, msg, wParam, lParam);
        // Lăsăm și procedura originală să proceseze, nu returnăm
    }

    // 6. TRATARE DEFAULT (Mesaje vizuale standard)
    // Dacă am ajuns aici și e WM_PAINT, înseamnă că m_showBorder e TRUE
    if (m_originalWndProc) {
        return CallWindowProc(m_originalWndProc, hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
*/

LRESULT vGroupBox::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // 1. CULORI PENTRU COPII (Redirecționare inteligentă)
    // Deoarece GroupBox-ul este el însuși un "copil" vizual, el nu ar trebui să decidă 
    // singur culorile, ci să lase sistemul tău de moștenire să urce la Panel.
    // 1. CULORI PENTRU COPII (Radio, Checkbox, Static, Edit)
    /*
    if (msg == WM_PAINT) {
        // 1. Desenarea standard (Rama)
        LRESULT res = CallWindowProc(m_originalWndProc, hwnd, msg, wParam, lParam);

        HDC hdc = GetDC(hwnd);
        SelectObject(hdc, getEffectiveFont());
        SetTextColor(hdc, getEffectiveTextColor());
        SetBkMode(hdc, TRANSPARENT);

        // 2. Calculăm dimensiunea textului pentru a nu-l tăia
        RECT textRect;
        GetClientRect(hwnd, &textRect);

        // Îi dăm o zonă generoasă în partea de sus
        textRect.left = 10;
        textRect.top = 0;
        textRect.right -= 10;
        textRect.bottom = 40; // Mărim de la 25 la 40 pentru siguranță

        // DT_NOCLIP previne tăierea textului dacă acesta depășește RECT-ul
        DrawTextW(hdc, m_title.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOCLIP);

        ReleaseDC(hwnd, hdc);
        return res;
    }
    */
    /*
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        // 1. Începem desenarea oficială. Asta validează zona și previne desenarea dublă de către Windows.
        HDC hdc = BeginPaint(hwnd, &ps);

        // Pregătim instrumentele de desenare
        COLORREF customColor = getEffectiveTextColor(); // Culoarea din XML (ex: Alb)
        HPEN hPen = CreatePen(PS_SOLID, 1, customColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        // Folosim o perie NULĂ pentru ca interiorul dreptunghiului să rămână transparent
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        // Luăm dimensiunile zonei Client (interioare)
        RECT rc;
        GetClientRect(hwnd, &rc);

        // 2. DESENĂM RAMA SIMPLĂ (Dreptunghiul)
        // Offset standard Win32 pentru ca linia de sus să treacă prin mijlocul textului
        int textHeightOffset = 10;
        Rectangle(hdc, rc.left, rc.top + textHeightOffset, rc.right, rc.bottom);

        // 3. DESENĂM TEXTUL (Titlul)
        // Selectăm fontul din XML
        SelectObject(hdc, getEffectiveFont());
        SetTextColor(hdc, customColor);

        // IMPORTANT: OPAQUE va folosi BkColor pentru a "șterge" linia de sub text
        SetBkMode(hdc, OPAQUE);

        // Luăm culoarea de fundal a GroupBox-ului pentru a o pune sub text
        SetBkColor(hdc, getEffectiveBackgroundColor());

        // Poziționăm textul (standard: 10px stânga, 0px sus)
        // DT_NOCLIP previne tăierea dacă fontul e puțin mai mare
        RECT textRect = { 10, 0, rc.right - 10, 25 };
        DrawTextW(hdc, m_title.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOCLIP);

        // Curățenie
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);

        // 4. Finalizăm desenarea
        EndPaint(hwnd, &ps);
        return 0; // Am desenat noi totul, oprim lanțul de mesaje
    }
    */
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // --- CODUL TĂU DE DESENARE RAMĂ ȘI TITLU (Păstrează-l pe cel de data trecută) ---
        COLORREF customColor = getEffectiveTextColor();
        HPEN hPen = CreatePen(PS_SOLID, 1, customColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RECT rc;
        GetClientRect(hwnd, &rc);
        Rectangle(hdc, rc.left, rc.top + 10, rc.right, rc.bottom);

        SelectObject(hdc, getEffectiveFont());
        SetTextColor(hdc, customColor);
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, getEffectiveBackgroundColor());
        RECT textRect = { 10, 0, rc.right - 10, 25 };
        DrawTextW(hdc, m_title.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOCLIP);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
        // -------------------------------------------------------------------------

        EndPaint(hwnd, &ps);

        // ACEASTA ESTE CHEIA: După ce am desenat rama, spunem containerului să-și facă treaba
        // dar FĂRĂ să mai lăsăm procedura originală (butonul standard) să deseneze.
        vContainer::handleMessage(hwnd, msg, wParam, lParam);

        //return 0;
    }
    if (msg == WM_CTLCOLORSTATIC || msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORBTN) {
        HWND hChild = (HWND)lParam;
        HDC hdc = (HDC)wParam;

        // Căutăm controlul în interiorul GroupBox-ului
        vControl* child = nullptr;
        for (auto& entry : m_children) {
            if (entry.second->getHandle() == hChild) {
                child = entry.second.get();
                break;
            }
        }

        if (child) {
            // APLICĂM MOȘTENIREA (Logica pe care am scris-o data trecută)
            // 1. Font
            SelectObject(hdc, child->getEffectiveFont());

            // 2. Text Color (Dacă nu are, ia de la GroupBox, care ia de la Panel)
            SetTextColor(hdc, child->getEffectiveTextColor());

            // 3. Background
            HBRUSH hBr = child->getEffectiveBackgroundBrush();
            if (hBr != GetStockObject(NULL_BRUSH)) {
                SetBkMode(hdc, OPAQUE);
                SetBkColor(hdc, child->getEffectiveBackgroundColor());
                return (LRESULT)hBr;
            }
            else {
                // Dacă nici copilul, nici GroupBox-ul nu au fundal setat,
                // cerem peria de la bunicul (Panel) ca să fim siguri că suntem transparenți pe fundalul corect
                SetBkMode(hdc, TRANSPARENT);
                return SendMessage(GetParent(hwnd), msg, wParam, lParam);
            }
        }
    }

    // 2. NOTIFICĂRI (BN_CLICKED de la Radio/Checkbox-uri din interior)
    // GroupBox-ul (BS_GROUPBOX) nu trimite notificări WM_COMMAND de la copiii săi mai departe.
    // Trebuie să le "împingem" noi manual către Panel.
    if (msg == WM_COMMAND || msg == WM_NOTIFY) {
        return SendMessage(GetParent(hwnd), msg, wParam, lParam);
    }

    // 3. RE-ROUTING PENTRU FONT
    // Dacă GroupBox primește un mesaj de schimbare font, trebuie să-l aplice și intern
    if (msg == WM_SETFONT) {
        m_hFont = (HFONT)wParam;
        // Nu returnăm, lăsăm procedura originală să seteze fontul titlului
    }

    // 4. LAYOUT & RESIZE
    if (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED) {
        vContainer::handleMessage(hwnd, msg, wParam, lParam);
        // Lăsăm și procedura originală (CallWindowProc)
    }

    // 5. LOGICA DE "TRANSPARENȚĂ" (ERASEBKGND)
    if (msg == WM_ERASEBKGND) {
        // Dacă GroupBox-ul nu are culoare custom, îi cerem părintelui să deseneze.
        // Asta previne acele pătrate gri urâte în jurul GroupBox-ului pe un Panel colorat.
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Întrebăm părintele (Panel) ce perie are
        HBRUSH hbr = (HBRUSH)SendMessage(GetParent(hwnd), WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hwnd);
        if (hbr) {
            FillRect(hdc, &rc, hbr);
            return 1;
        }
    }

    // 6. PROCEDURA ORIGINALĂ (Esențial pentru desenarea ramei și titlului)
    if (m_originalWndProc) {
        return CallWindowProc(m_originalWndProc, hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void vGroupBox::applyLayout() {
    if (m_layoutStrategy) {
        // Dacă layout-ul tău nu știe de margini interne, 
        // poți să "păcălești" strategia temporar aici.

        // Salvăm marginile vechi
        int oldTop = marginTop;

        // Adăugăm spațiul pentru titlu la marginea existentă
        marginTop += 25;

        m_layoutStrategy->applyLayout(*this);

        // Restaurăm marginea originală pentru a nu se cumula la resize
        marginTop = oldTop;
    }
}