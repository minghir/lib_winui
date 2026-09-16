#include "vPdfViewer.hpp"



vPdfViewer::vPdfViewer(HINSTANCE hInstance, EventDispatcher& dispatcher)
    : m_dispatcher(dispatcher) {
    m_pWindow = std::make_unique<vPdfViewerWindow>(hInstance, "pdf_viewer_app", dispatcher);
}

vPdfViewer::~vPdfViewer() {
    // Curățăm handlerele din EventDispatcher pentru controalele din acest viewer
    m_dispatcher.removeHandlers("btn_pdf_save_as");
    m_dispatcher.removeHandlers("btn_pdf_print");
    m_dispatcher.removeHandlers("btn_pdf_exit");
    m_dispatcher.removeHandlers("btn_pdf_zoom_in");
    m_dispatcher.removeHandlers("btn_pdf_zoom_out");
    m_dispatcher.removeHandlers("btn_pdf_zoom_reset");
    m_dispatcher.registerHandler("btn_pdf_next", WM_COMMAND, nullptr);
    m_dispatcher.registerHandler("btn_pdf_prev", WM_COMMAND, nullptr);
}

void vPdfViewer::buildMenu() {
    HWND hWin = m_pWindow->getHandle();

    // 1. Instanțiem submeniurile
    m_fileMenu = std::make_unique<vMenu>("pdf_file_menu", m_dispatcher);
    //m_fileMenu->addItem("btn_pdf_open", L"&Deschide PDF...\tCtrl+O");
    m_fileMenu->addItem("btn_pdf_save_as", L"&Salvează ca...\tCtrl+S");
    m_fileMenu->addItem("btn_pdf_print", L"&Tipărește...\tCtrl+P");
    m_fileMenu->addSeparator("sep1");
    m_fileMenu->addItem("btn_pdf_exit", L"Ieși&re");
    m_fileMenu->create(hWin); // ⭐ OBLIGATORIU: Creăm handle-ul Win32 mai întâi!

    m_viewMenu = std::make_unique<vMenu>("pdf_view_menu", m_dispatcher);
    m_viewMenu->addItem("btn_pdf_zoom_in", L"Mărește (&Zoom In)\tCtrl++");
    m_viewMenu->addItem("btn_pdf_zoom_out", L"Micșorează (&Zoom Out)\tCtrl+-");
    m_viewMenu->addItem("btn_pdf_zoom_reset", L"Dimensiune &Reală (100%)\tCtrl+0");
    m_viewMenu->create(hWin); // ⭐ Creăm handle-ul!

    m_navMenu = std::make_unique<vMenu>("pdf_nav_menu", m_dispatcher);
    m_navMenu->addItem("btn_pdf_next", L"Pagina &Următoare\tDreapta");
    m_navMenu->addItem("btn_pdf_prev", L"Pagina &Anterioară\tStânga");
    m_navMenu->create(hWin); // ⭐ Creăm handle-ul!

    // 2. Creăm Bara Principală de Meniu
    m_mainMenu = std::make_unique<vMenu>("pdf_main_menu", m_dispatcher);
    m_mainMenu->create(hWin); // ⭐ Creăm bara principală de meniu!

    // 3. Acum adăugăm submeniurile (toate au m_handle valid acum!)
    m_mainMenu->addSubMenu(L"&Fișier", m_fileMenu.get());
    m_mainMenu->addSubMenu(L"&Vizualizare", m_viewMenu.get());
    m_mainMenu->addSubMenu(L"&Navigare", m_navMenu.get());
}
/*
void vPdfViewer::registerEvents() {
    // Înregistrăm acțiunile direct pe ID-ul Fiecărui item de meniu la mesajul WM_COMMAND:

    // --- Meniu Fișier ---
    m_dispatcher.registerHandler("btn_pdf_exit", WM_COMMAND, [this]() {
        if (m_pWindow) m_pWindow->close();
        });

    m_dispatcher.registerHandler("btn_pdf_open", WM_COMMAND, [this]() {
        // Logica pentru deschidere fișier nou dacă este cazul
        });

    m_dispatcher.registerHandler("btn_pdf_save_as", WM_COMMAND, [this]() {
        if (m_pWindow) {
            m_pWindow->savePdfAs();
        }
        });

    m_dispatcher.registerHandler("btn_pdf_print", WM_COMMAND, [this]() {
        // Logica de printare
        });

    // --- Meniu Vizualizare (Zoom) ---
    m_dispatcher.registerHandler("btn_pdf_zoom_in", WM_COMMAND, [this]() {
        if (m_pWindow) m_pWindow->zoomIn();
        });

    m_dispatcher.registerHandler("btn_pdf_zoom_out", WM_COMMAND, [this]() {
        if (m_pWindow) m_pWindow->zoomOut();
        });

    m_dispatcher.registerHandler("btn_pdf_zoom_reset", WM_COMMAND, [this]() {
        if (m_pWindow) m_pWindow->resetZoom();
        });

    // --- Meniu Navigare ---
    m_dispatcher.registerHandler("btn_pdf_next", WM_COMMAND, [this]() {
        if (m_pWindow) m_pWindow->nextPage();
        });

    m_dispatcher.registerHandler("btn_pdf_prev", WM_COMMAND, [this]() {
        if (m_pWindow) m_pWindow->prevPage();
        });

    m_dispatcher.registerHandler("btn_pdf_print", WM_COMMAND, [this]() {
        if (m_pWindow) {
            m_pWindow->printCurrentDocument();
        }
        });
}
*/

void vPdfViewer::registerEvents() {
    // --- Meniu Fișier ---
    m_dispatcher.registerHandler("btn_pdf_exit", WM_COMMAND, [this]() {
        if (m_windowPtr) m_windowPtr->close();
        });

    m_dispatcher.registerHandler("btn_pdf_open", WM_COMMAND, [this]() {
        // Logica pentru deschidere fișier nou
        });

    m_dispatcher.registerHandler("btn_pdf_save_as", WM_COMMAND, [this]() {
        if (m_windowPtr) {
            m_windowPtr->savePdfAs();
        }
        });

    // --- Meniu Vizualizare (Zoom) ---
    m_dispatcher.registerHandler("btn_pdf_zoom_in", WM_COMMAND, [this]() {
        if (m_windowPtr) m_windowPtr->zoomIn();
        });

    m_dispatcher.registerHandler("btn_pdf_zoom_out", WM_COMMAND, [this]() {
        if (m_windowPtr) m_windowPtr->zoomOut();
        });

    m_dispatcher.registerHandler("btn_pdf_zoom_reset", WM_COMMAND, [this]() {
        if (m_windowPtr) m_windowPtr->resetZoom();
        });

    // --- Meniu Navigare ---
    m_dispatcher.registerHandler("btn_pdf_next", WM_COMMAND, [this]() {
        if (m_windowPtr) m_windowPtr->nextPage();
        });

    m_dispatcher.registerHandler("btn_pdf_prev", WM_COMMAND, [this]() {
        if (m_windowPtr) m_windowPtr->prevPage();
        });

    m_dispatcher.registerHandler("btn_pdf_print", WM_COMMAND, [this]() {
        if (m_windowPtr) {
            m_windowPtr->printCurrentDocument();
        }
        });
}

bool vPdfViewer::create(HWND parent, const std::wstring& title) {
    // 1. Creăm mai întâi fereastra nativă Win32
    bool ok = m_pWindow->create(
        L"VPdfViewerClass",
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 700,
        parent
    );

    if (ok) {
        m_windowPtr = m_pWindow.get();
        // 2. Construim meniurile (buildMenu folosește m_pWindow->getHandle())
        buildMenu();

        // 3. Înregistrăm handler-ele pentru evenimente
        registerEvents();

        // 4. Atașăm bara de meniu principală la fereastră
        m_pWindow->setMenu(m_mainMenu.get());
    }

    return ok;
}

bool vPdfViewer::loadFromMemory(const std::vector<uint8_t>& buffer) {
    return m_pWindow->loadPdfFromMemory(buffer);
}

bool vPdfViewer::loadFromFile(const std::wstring& filePath) {
    return m_pWindow->loadPdfFromFile(filePath);
}

std::unique_ptr<vPdfViewerWindow>vPdfViewer::extractWindow() {
    return std::move(m_pWindow);
}