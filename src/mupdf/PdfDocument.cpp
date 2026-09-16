#include "PdfDocument.hpp"
#include "..\stringUtils.hpp"
#include <stdexcept>
#include <cmath>

extern "C" {
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
}

PdfDocument::PdfDocument() {
    m_ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    if (!m_ctx) {
        throw std::runtime_error("Nu s-a putut crea contextul fz_context!");
    }
    fz_register_document_handlers(m_ctx);
}

PdfDocument::~PdfDocument() {
    close();
    if (m_ctx) {
        fz_drop_context(m_ctx);
        m_ctx = nullptr;
    }
}

bool PdfDocument::openFromMemory(const std::vector<uint8_t>& data, const std::string& name) {
    close();

    if (data.empty()) return false;

    // Salvează datele în bufferul persistent al obiectului C++
    m_memoryBuffer = data;

    fz_stream* stream = fz_open_memory(m_ctx, m_memoryBuffer.data(), m_memoryBuffer.size());
    if (!stream) return false;

    fz_try(m_ctx) {
        m_doc = fz_open_document_with_stream(m_ctx, "pdf", stream);
    }
    fz_always(m_ctx) {
        fz_drop_stream(m_ctx, stream);
    }
    fz_catch(m_ctx) {
        m_doc = nullptr;
        return false;
    }

    return m_doc != nullptr;
}

void PdfDocument::close() {
    if (m_doc) {
        fz_drop_document(m_ctx, m_doc);
        m_doc = nullptr;
    }
    m_memoryBuffer.clear();
}

int PdfDocument::getPageCount() const {
    if (!m_doc) return 0;
    int count = 0;
    fz_try(m_ctx) {
        count = fz_count_pages(m_ctx, m_doc);
    }
    fz_catch(m_ctx) {
        count = 0;
    }
    return count;
}

fz_pixmap* PdfDocument::renderPage(int pageIndex, float resolution) {
    if (!m_doc || pageIndex < 0 || pageIndex >= getPageCount()) return nullptr;

    fz_pixmap* pixmap = nullptr;
    float zoom = resolution / 72.0f;
    fz_matrix ctm = fz_scale(zoom, zoom);

    fz_try(m_ctx) {
        pixmap = fz_new_pixmap_from_page_number(
            m_ctx, m_doc, pageIndex, ctm, fz_device_rgb(m_ctx), 0
        );
    }
    fz_catch(m_ctx) {
        pixmap = nullptr;
    }

    return pixmap;
}


bool PdfDocument::openFromFile(const std::wstring& filePath) {
    close(); // Închide orice document anterior

    if (filePath.empty()) return false;

    // Folosim utilitarul tău existent pentru conversia wstring -> UTF-8 string
    std::string utf8Path = wstr_to_str(filePath);
    // Sau: std::string utf8Path = wstring_to_utf8(filePath);

    fz_try(m_ctx) {
        m_doc = fz_open_document(m_ctx, utf8Path.c_str());
    }
    fz_catch(m_ctx) {
        m_doc = nullptr;
        return false;
    }

    return m_doc != nullptr;
}




bool PdfDocument::print(HWND ownerHwnd, const std::wstring& docTitle) {
    if (!isOpen() || getPageCount() <= 0) return false;

    PRINTDLGW pd = { 0 };
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = ownerHwnd;
    pd.Flags = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

    if (!PrintDlgW(&pd)) return false; // Utilizatorul a dat Cancel

    HDC hdcPrinter = pd.hDC;
    if (!hdcPrinter) {
        MessageBoxW(ownerHwnd, L"Nu s-a putut obține Contextul de Dispozitiv (DC) pentru imprimantă!", L"Eroare Imprimantă", MB_ICONERROR);
        return false;
    }

    DOCINFOW di = { 0 };
    di.cbSize = sizeof(DOCINFOW);
    di.lpszDocName = docTitle.c_str();

    if (StartDocW(hdcPrinter, &di) <= 0) {
        MessageBoxW(ownerHwnd, L"Nu s-a putut inițializa documentul de tipărire!", L"Eroare Imprimantă", MB_ICONERROR);
        DeleteDC(hdcPrinter);
        return false;
    }

    SetMapMode(hdcPrinter, MM_TEXT);

    int dpiX = GetDeviceCaps(hdcPrinter, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdcPrinter, LOGPIXELSY);
    int min_dpi = (dpiX < dpiY) ? dpiX : dpiY;
    float dpiScale = static_cast<float>(min_dpi) / 72.0f;

    fz_colorspace* rgb_cs = fz_device_rgb(m_ctx);
    int totalPages = getPageCount();

    for (int i = 0; i < totalPages; i++) {
        if (StartPage(hdcPrinter) <= 0) break;

        fz_matrix ctm = fz_scale(dpiScale, dpiScale);
        fz_pixmap* pix = nullptr;

        fz_try(m_ctx) {
            pix = fz_new_pixmap_from_page_number(m_ctx, m_doc, i, ctm, rgb_cs, 0);
        }
        fz_catch(m_ctx) {
            pix = nullptr;
        }

        if (!pix || !pix->samples) {
            if (pix) fz_drop_pixmap(m_ctx, pix);
            EndPage(hdcPrinter);
            break;
        }

        // --- 1. CALCUL PADDING GDI ȘI ALOCARE BUFFER ---
        int bytes_per_pixel = 3; // RGB 24-bit
        int mu_bytes_per_line = pix->w * bytes_per_pixel;

        int gdi_aligned_stride = (mu_bytes_per_line + 3) & ~3;
        int padding_bytes = gdi_aligned_stride - mu_bytes_per_line;

        size_t total_buffer_size = static_cast<size_t>(gdi_aligned_stride) * pix->h;
        unsigned char* gdi_buffer = static_cast<unsigned char*>(malloc(total_buffer_size));

        if (!gdi_buffer) {
            MessageBoxW(ownerHwnd, L"Eroare de alocare memorie pentru buffer-ul GDI!", L"Eroare Imprimantă", MB_ICONERROR);
            fz_drop_pixmap(m_ctx, pix);
            EndPage(hdcPrinter);
            break;
        }

        // --- 2. COPIERE LINIE CU LINIE + SWAP RGB -> BGR ȘI PADDING ---
        unsigned char* mu_src = pix->samples;
        unsigned char* gdi_dest = gdi_buffer;
        int mu_stride = pix->stride;

        for (int y = 0; y < pix->h; y++) {
            unsigned char* s = mu_src;
            unsigned char* d = gdi_dest;

            for (int x = 0; x < pix->w; x++) {
                d[0] = s[2]; // B
                d[1] = s[1]; // G
                d[2] = s[0]; // R
                s += pix->n;
                d += bytes_per_pixel;
            }

            if (padding_bytes > 0) {
                memset(gdi_dest + mu_bytes_per_line, 0, padding_bytes);
            }

            mu_src += mu_stride;
            gdi_dest += gdi_aligned_stride;
        }

        // --- 3. HEADER BITMAPINFO ---
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = pix->w;
        bmi.bmiHeader.biHeight = -pix->h; // Top-Down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        // --- 4. SCALARE PROPORȚIONALĂ ȘI STRETCHDIBITS ---
        int image_w = pix->w;
        int image_h = pix->h;
        int print_area_width = GetDeviceCaps(hdcPrinter, HORZRES);
        int print_area_height = GetDeviceCaps(hdcPrinter, VERTRES);

        float scale_w = static_cast<float>(print_area_width) / image_w;
        float scale_h = static_cast<float>(print_area_height) / image_h;
        float scale = (scale_w < scale_h) ? scale_w : scale_h;

        int final_w = static_cast<int>(std::round(image_w * scale));
        int final_h = static_cast<int>(std::round(image_h * scale));

        int x_offset = (print_area_width - final_w) / 2;
        int y_offset = (print_area_height - final_h) / 2;

        SetStretchBltMode(hdcPrinter, HALFTONE);

        int result = StretchDIBits(
            hdcPrinter,
            x_offset, y_offset, final_w, final_h,
            0, 0, image_w, image_h,
            gdi_buffer,
            &bmi,
            DIB_RGB_COLORS,
            SRCCOPY
        );

        free(gdi_buffer);
        fz_drop_pixmap(m_ctx, pix);

        if (result == GDI_ERROR) {
            MessageBoxW(ownerHwnd, L"StretchDIBits a returnat GDI_ERROR!", L"Eroare GDI", MB_ICONERROR);
        }

        if (EndPage(hdcPrinter) <= 0) break;
    }

    EndDoc(hdcPrinter);
    DeleteDC(hdcPrinter);
    return true;
}

bool PdfDocument::saveToFile(const std::wstring& filePath) {
    if (!isOpen() || filePath.empty()) return false;

    std::string utf8Path = wstr_to_str(filePath);

    pdf_document* pdfDoc = pdf_specifics(m_ctx, m_doc);
    if (!pdfDoc) return false;

    fz_try(m_ctx) {
        // Opțiune varianta A: Atribuire directă din macro-ul/structura globală
        pdf_write_options opts = pdf_default_write_options;

        pdf_save_document(m_ctx, pdfDoc, utf8Path.c_str(), &opts);
    }
    fz_catch(m_ctx) {
        return false;
    }

    return true;
}


