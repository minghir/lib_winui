#include "ui/ConsoleManager.hpp"
#include "ui/vApp.hpp"

#include "stringUtils.hpp"
#include "ConfigLoader.hpp"


//#include "reportHandler.hpp"
//#include "multiReport.hpp"
#include "reportUtils.hpp"
#include "fileUtils.hpp"
#include "mupdf/win_diag.h"



#include<iostream>
#include <sstream>
#include <filesystem>
#include <string>
#include <chrono>
#include <iomanip>
#include <conio.h> 


class myApp : public vApp {
private:
    std::unique_ptr<vPdfViewer> m_pdfViewerKeeper; // Menține în viață meniurile și evenimentele

public:
    myApp(HINSTANCE hInstance, RunMode rm) : vApp(hInstance) { setRunMode(rm); }
    ~myApp() {}

    bool initGui() override {
        LOG_INFO(L"START CONVERSION");

        std::wstring rtfFileFullPath = L"tmp/oper_invoice_202608";
        std::wstring pdfFileFullPath = rtfFileFullPath + L".pdf";

        LOG_DEBUG(L"Fișier RTF salvat la: " + rtfFileFullPath);
        LOG_DEBUG(L"Fișier PDF de generat la: " + pdfFileFullPath);

        // 1. Conversie RTF -> PDF pe HDD
        bool succes = tdocsRTFtoPDF(rtfFileFullPath, L"");
        if (!succes) {
            LOG_ERROR(L"Conversie la pdf esuata");
            return false;
        }

        HINSTANCE hInst = GetModuleHandle(NULL);

        // 2. Instanțiem vPdfViewer direct
        m_pdfViewerKeeper = std::make_unique<vPdfViewer>(hInst, getEventDispatcher());

        // 3. Creăm fereastra ca top-level (parent = nullptr), devenind fereastra principală
        if (!m_pdfViewerKeeper->create(nullptr, L"Raport ANC PDF")) {
            LOG_ERROR(L"Nu s-a putut crea fereastra PDF Viewer.");
            return false;
        }

        // 4. Citim fișierul PDF în memorie și îl încărcăm în viewer
        std::ifstream file(pdfFileFullPath, std::ios::binary | std::ios::ate);
        bool loaded = false;
        if (file.is_open()) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> buffer(size);
            if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                file.close();
                loaded = m_pdfViewerKeeper->loadFromMemory(buffer);
            }
            else {
                file.close();
            }
        }

        // Fallback la încărcarea directă de pe disc dacă memoria a eșuat
        if (!loaded) {
            loaded = m_pdfViewerKeeper->loadFromFile(pdfFileFullPath);
        }

        if (!loaded) {
            LOG_ERROR(L"Nu s-a putut încărca documentul PDF în viewer.");
            return false;
        }

        // 5. 🔥 AICI ESTE MAGIA: Înregistrăm fereastra PDF direct ca "main" în vApp.
        // Nicio fereastră în plus, nicio fereastră ascunsă!
        addWindow("main", m_pdfViewerKeeper->extractWindow());
        return true;
    }
};


int main(int argc, char* argv[]) {
    //myApp app(NULL, RunMode::CONSOLE);
    myApp app(NULL, RunMode::GUI);
    app.startConsole();
    return app.run();
}