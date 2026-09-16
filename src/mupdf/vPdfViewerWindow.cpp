#include "vPdfViewerWindow.hpp"
#include "../ui/ControlIdManager.hpp"

extern "C" {
#include <mupdf/fitz.h>
}

vPdfViewerWindow::vPdfViewerWindow(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher)
    : vWindow(hInstance, id, WindowType::StandardWindow, false, dispatcher) {
}

vPdfViewerWindow::~vPdfViewerWindow() {
    freeCurrentPixmap();
}

void vPdfViewerWindow::freeCurrentPixmap() {
    if (m_currentPixmap && m_pdfDoc.getContext()) {
        fz_drop_pixmap(m_pdfDoc.getContext(), m_currentPixmap);
        m_currentPixmap = nullptr;
    }
}

void vPdfViewerWindow::renderCurrentPage() {
    freeCurrentPixmap();
    if (m_pdfDoc.isOpen()) {
        // Obținem DPI-ul dinamic. Dacă avem HWND valid, folosim API-ul modern
        if (m_handle && IsWindow(m_handle)) {
            typedef UINT(WINAPI* GetDpiForWindowFn)(HWND);
            HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
            if (hUser32) {
                GetDpiForWindowFn pGetDpi = (GetDpiForWindowFn)GetProcAddress(hUser32, "GetDpiForWindow");
                if (pGetDpi) {
                    m_dpi = static_cast<float>(pGetDpi(m_handle));
                }
            }
        }

        if (m_dpi <= 0.0f) {
            HDC hdcScreen = GetDC(NULL);
            int sysDpi = GetDeviceCaps(hdcScreen, LOGPIXELSX);
            ReleaseDC(NULL, hdcScreen);
            m_dpi = (sysDpi > 0) ? static_cast<float>(sysDpi) : 96.0f;
        }

        // Resetează poziția de Pan la re-randare (schimbare pagină sau zoom)
        m_offsetX = 0;
        m_offsetY = 0;

        // Calculăm DPI-ul efectiv prin înmulțire cu factorul de zoom
        float effectiveDpi = m_dpi * m_zoomFactor;

        // Renderăm pagina cu DPI-ul scalat de zoom
        m_currentPixmap = m_pdfDoc.renderPage(m_currentPage, effectiveDpi);

        // Forțăm redesenarea suprafeței GDI
        if (m_handle && IsWindow(m_handle)) {
            InvalidateRect(m_handle, NULL, TRUE);
        }
    }
}

LRESULT vPdfViewerWindow::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

        // --- MOUSE PAN / DRAG LOGIC ---
    case WM_LBUTTONDOWN: {
        m_isDragging = true;
        m_lastMousePos.x = LOWORD(lParam);
        m_lastMousePos.y = HIWORD(lParam);
        SetCapture(hwnd);
        SetCursor(LoadCursor(NULL, IDC_SIZEALL)); // Cursor cu 4 săgeți
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (m_isDragging) {
            int currentX = LOWORD(lParam);
            int currentY = HIWORD(lParam);

            int deltaX = currentX - m_lastMousePos.x;
            int deltaY = currentY - m_lastMousePos.y;

            m_offsetX += deltaX;
            m_offsetY += deltaY;

            m_lastMousePos.x = currentX;
            m_lastMousePos.y = currentY;

            InvalidateRect(hwnd, NULL, FALSE); // Redesenare fără curățare completă
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (m_isDragging) {
            m_isDragging = false;
            ReleaseCapture();
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        return 0;
    }

                     // --- 1. SCURTĂTURI DE TASTATURĂ ---
    case WM_KEYDOWN: {
        bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        if (isCtrlPressed) {
            switch (wParam) {
            case 'P': // Ctrl + P -> Tipărire
                printCurrentDocument();
                return 0;
            case VK_ADD:        // Numpad +
            case 0xBB:          // '+' / '='
                zoomIn();
                return 0;

            case VK_SUBTRACT:   // Numpad -
            case 0xBD:          // '-' / '_'
                zoomOut();
                return 0;

            case '0':           // Ctrl + 0 (Reset zoom)
            case VK_NUMPAD0:
                resetZoom();
                return 0;

            case VK_PRIOR:      // Ctrl + PgUp
                goToPage(1);
                return 0;

            case VK_NEXT:       // Ctrl + PgDn
                goToPage(getTotalPages());
                return 0;
            }
        }
        else {
            switch (wParam) {
            case VK_PRIOR:      // Page Up
            case VK_UP:         // Săgeată Sus
                prevPage();
                return 0;

            case VK_NEXT:       // Page Down
            case VK_DOWN:       // Săgeată Jos
            case VK_SPACE:      // Space
                nextPage();
                return 0;

            case VK_HOME:       // Home
                goToPage(1);
                return 0;

            case VK_END:        // End
                goToPage(getTotalPages());
                return 0;
            }
        }
        break;
    }

                   // --- 2. CTRL + MOUSE WHEEL ---
    case WM_MOUSEWHEEL: {
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);

        if (fwKeys & MK_CONTROL) {
            if (zDelta > 0) {
                zoomIn();
            }
            else {
                zoomOut();
            }
            return 0;
        }
        else {
            if (zDelta > 0) {
                prevPage();
            }
            else {
                nextPage();
            }
            return 0;
        }
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        std::string controlId = ControlIdManager::getNameById(wmId);

        if (!controlId.empty() && controlId != "<unknown>") {
            m_dispatcher.dispatch(controlId, WM_COMMAND);
            return 0;
        }
        break;
    }

    case 0x02E0: // WM_DPICHANGED
    {
        WORD newDpi = HIWORD(wParam);
        m_dpi = static_cast<float>(newDpi);

        renderCurrentPage();

        RECT* const prcNewWindow = (RECT*)lParam;
        SetWindowPos(hwnd,
            NULL,
            prcNewWindow->left,
            prcNewWindow->top,
            prcNewWindow->right - prcNewWindow->left,
            prcNewWindow->bottom - prcNewWindow->top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_SIZE: {
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int win_w = clientRect.right - clientRect.left;
        int win_h = clientRect.bottom - clientRect.top;

        // --- DOUBLE BUFFERING SETUP ---
        // Creăm un DC compatibil în memorie (off-screen)
        HDC memDC = CreateCompatibleDC(hdc);
        // Creăm un bitmap compatibil cu suprafața ferestrei curente
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, win_w, win_h);
        // Selectăm bitmap-ul în DC-ul de memorie
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // 1. Curățăm fundalul pe MEMORY DC
        HBRUSH bgBrush = CreateSolidBrush(RGB(112, 112, 112));
        FillRect(memDC, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        if (m_currentPixmap) {
            fz_context* ctx = m_pdfDoc.getContext();
            int iw = fz_pixmap_width(ctx, m_currentPixmap);
            int ih = fz_pixmap_height(ctx, m_currentPixmap);
            int n = fz_pixmap_components(ctx, m_currentPixmap);
            unsigned char* samples = fz_pixmap_samples(ctx, m_currentPixmap);
            int mu_stride = fz_pixmap_stride(ctx, m_currentPixmap);

            if (samples && iw > 0 && ih > 0) {
                // Calcul poziție (bază centrată + offset de la Pan)
                int base_x = (win_w > iw) ? (win_w - iw) / 2 : 0;
                int base_y = (win_h > ih) ? (win_h - ih) / 2 : 0;

                int dest_x = base_x + m_offsetX;
                int dest_y = base_y + m_offsetY;

                // Calcul padding GDI
                int bytes_per_pixel = (n >= 3) ? n : 3;
                int mu_bytes_per_line = iw * bytes_per_pixel;
                int gdi_aligned_stride = (mu_bytes_per_line + 3) & ~3;
                int padding_bytes = gdi_aligned_stride - mu_bytes_per_line;

                size_t total_gdi_buffer_size = static_cast<size_t>(gdi_aligned_stride) * ih;
                unsigned char* gdi_buffer = static_cast<unsigned char*>(malloc(total_gdi_buffer_size));

                if (gdi_buffer) {
                    unsigned char* mu_src = samples;
                    unsigned char* gdi_dest = gdi_buffer;

                    for (int y = 0; y < ih; y++) {
                        unsigned char* s = mu_src;
                        unsigned char* d = gdi_dest;

                        for (int x = 0; x < iw; x++) {
                            if (n >= 3) {
                                d[0] = s[2]; // B
                                d[1] = s[1]; // G
                                d[2] = s[0]; // R
                                if (n == 4) d[3] = s[3];
                            }
                            else if (n == 1) { // Grayscale
                                d[0] = s[0];
                                d[1] = s[0];
                                d[2] = s[0];
                            }
                            s += n;
                            d += bytes_per_pixel;
                        }

                        if (padding_bytes > 0) {
                            memset(gdi_dest + mu_bytes_per_line, 0, padding_bytes);
                        }

                        mu_src += mu_stride;
                        gdi_dest += gdi_aligned_stride;
                    }

                    BITMAPINFO bmi = { 0 };
                    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bmi.bmiHeader.biWidth = iw;
                    bmi.bmiHeader.biHeight = -ih;
                    bmi.bmiHeader.biPlanes = 1;
                    bmi.bmiHeader.biBitCount = static_cast<WORD>(bytes_per_pixel * 8);
                    bmi.bmiHeader.biCompression = BI_RGB;

                    // 2. Desenăm PDF-ul tot pe MEMORY DC
                    SetDIBitsToDevice(
                        memDC,
                        dest_x, dest_y, iw, ih,
                        0, 0, 0, ih,
                        gdi_buffer,
                        &bmi,
                        DIB_RGB_COLORS
                    );

                    free(gdi_buffer);
                }
            }
        }

        // 3. Copiem rezultatul final din Memory DC pe DC-ul ecranului într-o singură operație (BitBlt)
        BitBlt(hdc, 0, 0, win_w, win_h, memDC, 0, 0, SRCCOPY);

        // --- DOUBLE BUFFERING CLEANUP ---
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY: {
        // Resetăm handle-ul ferestrei
        m_handle = NULL;
        break;
    }
    }


    return vWindow::handleMessage(hwnd, msg, wParam, lParam);
}

bool vPdfViewerWindow::setupLoadedDocument() {
    m_currentPage = 0;
    renderCurrentPage();

    if (!m_currentPixmap) {
        return false;
    }

    int img_w = fz_pixmap_width(m_pdfDoc.getContext(), m_currentPixmap);
    int img_h = fz_pixmap_height(m_pdfDoc.getContext(), m_currentPixmap);

    if (m_handle && IsWindow(m_handle)) {
        RECT workArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
        int max_w = static_cast<int>((workArea.right - workArea.left) * 0.85f);
        int max_h = static_cast<int>((workArea.bottom - workArea.top) * 0.85f);

        int target_w = (img_w < max_w) ? img_w : max_w;
        int target_h = (img_h < max_h) ? img_h : max_h;

        RECT winRect = { 0, 0, target_w, target_h };
        DWORD style = GetWindowLong(m_handle, GWL_STYLE);
        BOOL hasMenu = (GetMenu(m_handle) != NULL);
        AdjustWindowRect(&winRect, style, hasMenu);

        int final_w = winRect.right - winRect.left;
        int final_h = winRect.bottom - winRect.top;

        int pos_x = workArea.left + ((workArea.right - workArea.left) - final_w) / 2;
        int pos_y = workArea.top + ((workArea.bottom - workArea.top) - final_h) / 2;

        SetWindowPos(m_handle, NULL, pos_x, pos_y, final_w, final_h, SWP_NOZORDER | SWP_SHOWWINDOW);
        InvalidateRect(m_handle, NULL, TRUE);
    }

    return true;
}

bool vPdfViewerWindow::loadPdfFromMemory(const std::vector<uint8_t>& buffer) {
    if (!m_pdfDoc.openFromMemory(buffer)) return false;
    return setupLoadedDocument();
}

bool vPdfViewerWindow::loadPdfFromFile(const std::wstring& filePath) {
    if (!m_pdfDoc.openFromFile(filePath)) return false;
    return setupLoadedDocument();
}


void vPdfViewerWindow::nextPage() {
    if (m_currentPage + 1 < m_pdfDoc.getPageCount()) {
        m_currentPage++;
        renderCurrentPage();
    }
}

void vPdfViewerWindow::prevPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        renderCurrentPage();
    }
}

void vPdfViewerWindow::goToPage(int page) {
    if (page >= 1 && page <= m_pdfDoc.getPageCount()) {
        m_currentPage = page - 1;
        renderCurrentPage();
    }
}

void vPdfViewerWindow::zoomIn() {
    if (m_zoomFactor < 4.0f) { // Limită max 400%
        m_zoomFactor += 0.15f;
        renderCurrentPage();
    }
}

void vPdfViewerWindow::zoomOut() {
    if (m_zoomFactor > 0.25f) { // Limită min 25%
        m_zoomFactor -= 0.15f;
        renderCurrentPage();
    }
}

void vPdfViewerWindow::setZoom(float factor) {
    if (factor >= 0.25f && factor <= 4.0f) {
        m_zoomFactor = factor;
        renderCurrentPage();
    }
}

void vPdfViewerWindow::resetZoom() {
    m_zoomFactor = 1.0f;
    renderCurrentPage();
}


bool vPdfViewerWindow::savePdfAs() {
    if (!m_pdfDoc.isOpen()) return false;

    // 1. Salvăm directorul de lucru curent
    wchar_t currentDir[MAX_PATH] = { 0 };
    GetCurrentDirectoryW(MAX_PATH, currentDir);

    wchar_t szFile[MAX_PATH] = L"Raport.pdf";

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_handle;
    ofn.lpstrFilter = L"Fișiere PDF (*.pdf)\0*.pdf\0Toate Fișierele (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"pdf";

    BOOL result = GetSaveFileNameW(&ofn);

    // 2. Restaurăm OBLIGATORIU directorul de lucru inițial
    SetCurrentDirectoryW(currentDir);

    if (result) {
        if (m_pdfDoc.saveToFile(szFile)) {
            MessageBoxW(m_handle, L"Raportul a fost salvat cu succes!", L"Succes", MB_OK | MB_ICONINFORMATION);
            return true;
        }
        else {
            MessageBoxW(m_handle, L"Eroare la salvarea fișierului PDF.", L"Eroare", MB_OK | MB_ICONERROR);
        }
    }

    return false;
}