#include "reportUtils.hpp"
#include "fileUtils.hpp"
#include "stringUtils.hpp"
#include "globals.hpp"
#include "tdocs/rtf.hpp"
#include "tdocs/RTFtoPDFConverter.hpp"
#include "tdocs/Xhtml.hpp"
#include "tdocs/PdfConverter.hpp"
#include "mupdf/fitz.h"


#include <PDFPageInput.h>



//#include "PdfConverter.hpp"


#include <PDFWriter.h>
#include <PDFDocumentCopyingContext.h>
#include <PDFPage.h>
#include <PageContentContext.h>
#include <PDFUsedFont.h>
#include <AbstractContentContext.h>
#include "PDFParser.h"
#include <memory>


//#include <sys/types.h>
//#include <sched.h>
//#include <exception>

#include <fstream>
//nclude <iostream>

//#include <cstdlib>
#include <iostream>
//#include <string>
#include <windows.h>


namespace fs = std::filesystem;
/*
void create_dir_if_missing(const std::string& path) {
    if (!fs::exists(path)) {
        if (fs::create_directory(path)) {
            std::wcout << L"Directorul \"" << str_to_wstr(path) << L"\" a fost creat cu succes.\n";
        }
        else {
            std::cerr << "Eroare la crearea directorului \"" << path << "\".\n";
        }
    }
    else {
        std::wcout << L"Directorul \"" << str_to_wstr(path) << L"\" există deja.\n";
    }
}


*/
/*
std::wstring wstr_read_RTF_file(const std::string& filePath) {
    std::wifstream file(filePath, std::ios::in);
    if (!file.is_open()) {
        std::wcerr << L"Eroare: Nu s-a putut deschide fișierul " << str_to_wstr(filePath) << std::endl;
        return L""; // Returnează un string gol dacă fișierul nu se deschide
    }

    std::wstring content, line;
    while (std::getline(file, line)) {
        content += line + L"\n"; // Adaugă fiecare linie în conținut
    }

    file.close();
    return content;
}
*/
/*
std::wstring wstr_read_RTF_file(const std::string& filePath) {
    std::string path = getGlobalReportPath() + filePath;
    //std::string path =  filePath;

    //MessageBox(NULL, str_to_wstr(path).c_str(), L"Întrebare", MB_YESNO | MB_ICONQUESTION);


    std::wifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::wcerr << L"Eroare: Nu s-a putut deschide fisierul " << str_to_wstr(filePath) << std::endl;
        return L""; // Returnează un string gol dacă fișierul nu se deschide
    }

    std::wstring content, line;
    while (std::getline(file, line)) {
        content += line + L"\n"; // Adaugă fiecare linie în conținut
    }

    file.close();
    return content;
}
*/

/*
void convertRTFtoPDF(const std::string& rtfFile, const std::string& pdfDir) {
    std::string command = "LibreOfficePortable\\App\\libreoffice\\program\\soffice.exe --headless --convert-to pdf \""
                          + rtfFile + "\" --outdir \"" + pdfDir + "\"";
    std::system(command.c_str());
}
*/
/*

void convertRTFtoPDF(const std::string& rtfFile, const std::string& pdfDir) {
    std::string command = "LibreOfficePortable\\App\\libreoffice\\program\\soffice.exe --headless --convert-to pdf \""
        + rtfFile + "\" --outdir \"" + pdfDir + "\"";
    std::wcout << L"Execut:" << str_to_wstr(command) << std::endl;
    int exitCode = std::system(command.c_str());

    if (exitCode == 0) {
        std::wcout << L"Conversia RTF -> PDF s-a terminat cu succes!\n";
    }
    else {
        std::cerr << L"Eroare la conversie! Cod de ieșire: " << exitCode << "\n";
    }
}


void wconvertRTFtoPDF(const std::string& rtfFile, const std::string& pdfDir) {
    std::string command = "LibreOfficePortable\\App\\libreoffice\\program\\soffice.exe --headless --convert-to pdf \""
        + rtfFile + "\" --outdir \"" + pdfDir + "\"";

    std::wstring wcommand = str_to_wstr(command);

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.cb = sizeof(si); // Trebuie setat explicit

    PROCESS_INFORMATION pi = {};


    si.cb = sizeof(si);

    if (CreateProcessW(nullptr, &wcommand[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        MessageBoxW(nullptr, L"Eroare la rularea LibreOffice!", L"Eroare", MB_OK | MB_ICONERROR);
    }
}
*/
/*
void initLibreOffice() {
    std::string command = "LibreOfficePortable\\App\\libreoffice\\program\\soffice --headless --accept=\"socket,host=localhost,port=8100;urp;\" &";

    std::wstring wcommand = str_to_wstr(command);

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.cb = sizeof(si); // Trebuie setat explicit

    PROCESS_INFORMATION pi = {};


    si.cb = sizeof(si);

    if (CreateProcessW(nullptr, &wcommand[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        MessageBoxW(nullptr, L"Eroare la rularea LibreOffice!", L"Eroare", MB_OK | MB_ICONERROR);
    }
}
*/

/*
bool copyFile(const std::string& source, const std::string& destination) {
    try {
        fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
        return true;  // Copiere reușită
    }
    catch (const std::exception& e) {
        std::cerr << "Eroare la copiere: " << e.what() << std::endl;
        return false;  // Copiere eșuată
    }
}

bool removeFile(const std::string& filePath) {
    std::filesystem::path path(filePath);
    return std::filesystem::remove(path);
}
*/


bool tdocsRTFtoPDF(const std::wstring& rtfFile, const std::wstring& pdfDir) {

    Rtf rtfDoc;

    if (rtfDoc.load(rtfFile + L".rtf")) {
        //AfxMessageBox(_T(wstr_to_str(rtfFile + L".rtf").c_str()));
        RtfToPdfConverter converter(rtfDoc);
        //AfxMessageBox(_T(wstr_to_str(pdfDir + rtfFile + L".pdf").c_str()));
        if (converter.convert(pdfDir + rtfFile + L".pdf")) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
    return true;


}

/*
bool tdocsXHTMLtoPDF(const std::wstring& xhtmlFile, const std::wstring& pdfDir) {



    Xhtml xhtmlDoc;

    if (xhtmlDoc.load(xhtmlFile + L".xhtml")) {


        PdfConverter pdfConv(xhtmlDoc);

        if (pdfConv.convert(pdfDir + xhtmlFile + L".pdf")) {
            pdfConv.clean();
            return true;
        } else {
            return false;
        }


    }
    else {
        return false;
    }
    return true;


}
*/

bool tdocsXHTMLtoPDF(const std::wstring& xhtmlFile, const std::wstring& pdfDir) {
    // 1. Verificăm dacă fișierul sursă există (folosind std::filesystem pentru siguranță)
    std::wstring sourceFile = xhtmlFile;
    if (sourceFile.find(L".xhtml") == std::wstring::npos) {
        sourceFile += L".xhtml";
    }

    Xhtml xhtmlDoc;
    if (!xhtmlDoc.load(sourceFile)) {
        LOG_ERROR(L"tdocsXHTMLtoPDF: Nu s-a putut incarca fisierul: " + sourceFile);
        return false;
    }

    // 2. Construim calea de ieșire
    // Dacă pdfDir e gol, salvăm în același loc cu sursa, dar cu extensia .pdf
    std::wstring outputFile;
    if (pdfDir.empty()) {
        outputFile = sourceFile;
        size_t lastDot = outputFile.find_last_of(L".");
        outputFile = outputFile.substr(0, lastDot) + L".pdf";
    }
    else {
        // Dacă avem director, ne asigurăm că avem slash la final
        std::wstring cleanDir = pdfDir;
        if (cleanDir.back() != L'\\' && cleanDir.back() != L'/') cleanDir += L"/";

        // Extragem doar numele fișierului din xhtmlFile (fără cale)
        std::filesystem::path p(xhtmlFile);
        outputFile = cleanDir + p.stem().wstring() + L".pdf";
    }

    PdfConverter pdfConv(xhtmlDoc);
    if (pdfConv.convert(outputFile)) {
        pdfConv.clean();
        LOG_SUCCESS(L"Raport PDF generat: " + outputFile);
        return true;
    }

    LOG_ERROR(L"tdocsXHTMLtoPDF: Conversia a esuat pentru: " + outputFile);
    return false;
}

bool startConsole() {
    ConsoleManager::getInstance().initialize();
    ConsoleManager::getInstance().setColor(FOREGROUND_GREEN);
    ConsoleManager::getInstance().log(L"Consola inițializată! [AppInit] Începe inițializarea aplicației...");
    ConsoleManager::getInstance().resetColor();
    return true;
}


/*
void wconcat_pdfs(const std::vector<std::wstring>& input_files,
    const std::wstring& output_file) {
    fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if (!ctx) {
        std::wcerr << L"Eroare la inițializarea contextului MuPDF\n";
        return;
    }

    fz_try(ctx) {
        fz_register_document_handlers(ctx);
        std::string output_utf8 = wstring_to_utf8(output_file);
        fz_document_writer* writer = fz_new_document_writer(ctx, output_utf8.c_str(), "pdf", nullptr);

        auto append_pdf = [&](const std::wstring& file) {
            std::string file_utf8 = wstring_to_utf8(file);
            fz_document* doc = fz_open_document(ctx, file_utf8.c_str());
            if (!doc) {
                std::wcerr << L"Nu s-a putut deschide fișierul: " << file << L"\n";
                return;
            }

            int page_count = fz_count_pages(ctx, doc);
            for (int i = 0; i < page_count; ++i) {
                fz_page* page = fz_load_page(ctx, doc, i);
                fz_rect mediabox = fz_bound_page(ctx, page);
                fz_device* dev = fz_begin_page(ctx, writer, mediabox);
                fz_run_page(ctx, page, dev, fz_identity, nullptr);
                fz_end_page(ctx, writer);
                fz_drop_page(ctx, page);
            }

            fz_drop_document(ctx, doc);
        };

        // parcurgem vectorul de fișiere
        for (const auto& file : input_files) {
            append_pdf(file);
        }

        fz_close_document_writer(ctx, writer);
        fz_drop_document_writer(ctx, writer);
    }
    fz_catch(ctx) {
        std::wcerr << L"Eroare în timpul procesării PDF-urilor\n";
    }

    fz_drop_context(ctx);
}
*/

void wconcat_pdfs(const std::vector<std::wstring>& input_files,
    const std::wstring& output_file)
{
    startConsole();
    // Context cu cache limitat (256 MB)
    fz_context* ctx = fz_new_context(nullptr, nullptr, 256 << 20);
    if (!ctx) {
        std::wcerr << L"Eroare la inițializarea contextului MuPDF\n";
        return;
    }

    fz_try(ctx) {
        fz_register_document_handlers(ctx);

        std::string output_utf8 = wstring_to_utf8(output_file);
        fz_document_writer* writer =
            fz_new_document_writer(ctx, output_utf8.c_str(), "pdf", nullptr);

        auto append_pdf = [&](const std::wstring& file) {
            std::string file_utf8 = wstring_to_utf8(file);
            fz_document* doc = fz_open_document(ctx, file_utf8.c_str());
            if (!doc) {
                std::wcerr << L"Nu s-a putut deschide fișierul: " << file << L"\n";
                return;
            }

            int page_count = fz_count_pages(ctx, doc);
            for (int i = 0; i < page_count; ++i) {
                fz_page* page = fz_load_page(ctx, doc, i);
                if (!page) {
                    std::wcerr << L"Eroare: pagina " << i << L" nu a putut fi încărcată\n";
                    continue;
                }


                fz_rect mediabox = fz_bound_page(ctx, page);

                fz_device* dev = fz_begin_page(ctx, writer, mediabox);
                if (!dev) {
                    std::wcerr << L"Nu s-a putut deschide fișierul: " << file << L"\n";
                    return;
                }

                fz_run_page(ctx, page, dev, fz_identity, nullptr);
                fz_end_page(ctx, writer);

                fz_drop_device(ctx, dev);   // 🔑 eliberăm device-ul
                fz_drop_page(ctx, page);    // 🔑 eliberăm pagina
            }

            fz_drop_document(ctx, doc);     // 🔑 eliberăm documentul
        };

        // parcurgem vectorul de fișiere
        for (const auto& file : input_files) {
            append_pdf(file);
        }

        fz_close_document_writer(ctx, writer);
        fz_drop_document_writer(ctx, writer);
    }
    fz_catch(ctx) {
        std::wcerr << L"Eroare în timpul procesării PDF-urilor\n";
    }

    fz_drop_context(ctx); // 🔑 eliberăm contextul
}

inline std::string wide_to_utf8_win32(const std::wstring& w) {
    if (w.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), out.data(), size, nullptr, nullptr);
    return out;
}


/*

void concat_pdfs(const std::vector<std::wstring>& inputFiles,
    const std::wstring& outputFile)
{
    std::string output_utf8 = wide_to_utf8_win32(outputFile);

    PDFWriter pdfWriter;
    EStatusCode status = pdfWriter.StartPDF(output_utf8, ePDFVersion13);
    if (status != eSuccess) {
        std::wcerr << L"Eroare la crearea fișierului de ieșire\n";
        return;
    }

    for (const auto& file : inputFiles) {
        if (_waccess(file.c_str(), 0) != 0) {
            std::wcerr << L"Fișier inexistent: " << file << L"\n";
            continue;
        }

        std::string file_utf8 = wide_to_utf8_win32(file);
        PDFDocumentCopyingContext* copyCtx = pdfWriter.CreatePDFCopyingContext(file_utf8);
        if (!copyCtx) {
            std::wcerr << L"Nu s-a putut crea contextul pentru " << file << L"\n";
            continue;
        }

        auto* parser = copyCtx->GetSourceDocumentParser();
        if (!parser) {
            std::wcerr << L"Parser invalid pentru: " << file << L"\n";
            delete copyCtx; // important în varianta A
            continue;
        }

        int pageCount = parser->GetPagesCount();
        for (int i = 0; i < pageCount; ++i) {
            EStatusCodeAndObjectIDType res = copyCtx->AppendPDFPageFromPDF(i);
            if (res.first != eSuccess) {
                std::wcerr << L"Eroare la copierea paginii " << i << L" din " << file << L"\n";
            }
        }

        delete copyCtx; // eliberezi explicit
    }

    pdfWriter.EndPDF();
}


void concat_pdfs(const std::vector<std::wstring>& inputFiles,
    const std::wstring& outputFile)
{
    std::string output_utf8 = wide_to_utf8_win32(outputFile);

    PDFWriter pdfWriter;
    EStatusCode status = pdfWriter.StartPDF(output_utf8, ePDFVersion13);
    if (status != eSuccess) {
        std::wcerr << L"Eroare la crearea fișierului de ieșire\n";
        return;
    }

    for (const auto& file : inputFiles) {
        if (_waccess(file.c_str(), 0) != 0) {
            std::wcerr << L"Fișier inexistent: " << file << L"\n";
            continue;
        }

        std::string file_utf8 = wide_to_utf8_win32(file);
        PDFDocumentCopyingContext* copyCtx = pdfWriter.CreatePDFCopyingContext(file_utf8);
        if (!copyCtx) {
            std::wcerr << L"Nu s-a putut crea contextul pentru " << file << L"\n";
            continue;
        }

        auto* parser = copyCtx->GetSourceDocumentParser();
        if (!parser) {
            std::wcerr << L"Parser invalid pentru: " << file << L"\n";
            continue;
        }

        int pageCount = parser->GetPagesCount();
        for (int i = 0; i < pageCount; ++i) {
            EStatusCodeAndObjectIDType res = copyCtx->AppendPDFPageFromPDF(i);
            if (res.first != eSuccess) {
                std::wcerr << L"Eroare la copierea paginii " << i
                    << L" din fișierul " << file << L"\n";
            }
        }

        // 🔑 NU mai faci delete aici!
    }

    pdfWriter.EndPDF();
}
*/



// 1. Deleter personalizat pentru obiectele Hummus create cu 'new'
struct PDFCopyContextDeleter {
    void operator()(PDFDocumentCopyingContext* ctx) const {
        if (ctx) {
            delete ctx;
        }
    }
};

// 2. Definirea tipului de pointer unic
using UniquePDFCopyContext = std::unique_ptr<PDFDocumentCopyingContext, PDFCopyContextDeleter>;


void concat_pdfs(const std::vector<std::wstring>& inputFiles,
    const std::wstring& outputFile)
{
    std::string output_utf8 = wide_to_utf8_win32(outputFile);

    PDFWriter pdfWriter;
    EStatusCode status = pdfWriter.StartPDF(output_utf8, ePDFVersion13);
    if (status != eSuccess) {
        std::wcerr << L"Eroare la crearea fișierului de ieșire: " << outputFile << L"\n";
        return;
    }

    for (const auto& file : inputFiles) {
        if (_waccess(file.c_str(), 0) != 0) {
            std::wcerr << L"Fișier inexistent: " << file << L"\n";
            continue;
        }

        std::string file_utf8 = wide_to_utf8_win32(file);

        // ⭐ SCHIMBARE CRUCIALĂ: Utilizarea std::unique_ptr
        UniquePDFCopyContext copyCtx(pdfWriter.CreatePDFCopyingContext(file_utf8));

        if (!copyCtx) {
            // Hummus returnează nullptr dacă nu poate citi fișierul/contextul
            std::wcerr << L"Nu s-a putut crea contextul pentru citirea: " << file << L"\n";
            continue;
        }

        auto* parser = copyCtx->GetSourceDocumentParser();
        if (!parser) {
            std::wcerr << L"Parser invalid pentru: " << file << L"\n";
            continue; // UniquePtr va șterge automat copyCtx la ieșirea din iterație
        }

        int pageCount = parser->GetPagesCount();
        for (int i = 0; i < pageCount; ++i) {
            // Hummus folosește indexare de la 0 pentru pagini
            EStatusCodeAndObjectIDType res = copyCtx->AppendPDFPageFromPDF(i);
            if (res.first != eSuccess) {
                std::wcerr << L"Eroare la copierea paginii " << i + 1 << L" din " << file << L"\n";
                // Continuăm cu următoarea pagină sau document, deși aceasta e o eroare serioasă.
            }
        }

        // UniquePtr eliberează automat copyCtx la sfârșitul fiecărei iterații.
    }

    // ⭐ TRATAREA ERORII LA FINALIZAREA PDF-ului
    EStatusCode end_status = pdfWriter.EndPDF();
    if (end_status != eSuccess) {
        std::wcerr << L"Eroare la finalizarea fișierului de ieșire. Fișierul ar putea fi corupt sau incomplet.\n";
    }
}