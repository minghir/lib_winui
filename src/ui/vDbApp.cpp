/*
#include "vDbApp.hpp"
#include "ConsoleManager.hpp"
#include "FontManager.hpp"
#include "../odbcConnection.hpp"
#include "../stringUtils.hpp"
#include "vPanel.hpp"
#include "vLabel.hpp"
#include "vDbFilteredGrid.hpp"
#include "vDbComboBox.hpp"

vDbApp::vDbApp(HINSTANCE hInstance) : vApp(hInstance) {
    // Constructorul
}

void vDbApp::addDbConnection(const std::string& name, std::unique_ptr<dbConnection> conn) {
    if (!conn) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbApp::addDbConnection: Încercare de a adăuga o conexiune NULL.");
        return;
    }

    if (!conn->isConnected()) {
        ConsoleManager::getInstance().log(L"[WARNING] vDbApp::addDbConnection: Se adaugă o conexiune neactivă cu numele '" + str_to_wstr(name) + L"'.");
    }

    m_dbConnections[name] = std::move(conn);
    ConsoleManager::getInstance().log(L"[LOG] vDbApp::addDbConnection: Conexiunea '" + str_to_wstr(name) + L"' a fost adăugată.");
}

dbConnection* vDbApp::getDbConnection(const std::string& name) {
    auto it = m_dbConnections.find(name);
    if (it != m_dbConnections.end()) {
        return it->second.get();
    }
    ConsoleManager::getInstance().log(L"[WARNING] vDbApp::getDbConnection: Conexiunea '" + str_to_wstr(name) + L"' nu a fost găsită.");
    return nullptr;
}

bool vDbApp::init() {
    if (!vApp::init()) {
        return false;
    }

    vWindow* pMainWindow = getWindow("main");
    if (!pMainWindow) {
        ConsoleManager::getInstance().log(L"[ERROR] Fereastra principală 'main' nu a fost găsită după vApp::init().");
        return false;
    }

    vPanel* pMainPanel = static_cast<vPanel*>(pMainWindow->getChild("mainPanel"));
    if (!pMainPanel) {
        ConsoleManager::getInstance().log(L"[ERROR] Panoul principal 'mainPanel' nu a fost creat.");
        return false;
    }

    // 1. Inițializează conexiunea la baza de date.
    auto myDb1 = std::make_unique<odbcConnection>("odbc", L"ANC_Server");
    if (myDb1->openDatabase()) {
        this->addDbConnection("ANC_Server", std::move(myDb1));
        ConsoleManager::getInstance().log(L"Baza de date 'ANC_Server' a fost deschisă și adăugată.");
    }
    else {
        ConsoleManager::getInstance().log(L"[ERROR] Nu s-a putut deschide baza de date 'ANC_Server'.");
    }

    dbConnection* db = getDbConnection("ANC_Server");
    if (!db || !db->isConnected()) {
        ConsoleManager::getInstance().log(L"[WARNING] Nu se poate inițializa UI-ul legat de bazele de date; conexiunea este invalidă.");
        return true; // Continuă cu inițializarea UI-ului care nu necesită DB
    }
    
    // 2. Creează `vDbFilteredGrid`.
    auto myDbGrid = std::make_unique<vDbFilteredGrid>(
        m_instance,
        "myDbGrid",
        10, 10, 780, 580,
        db
        );
    pMainPanel->addChild("myDbGrid", std::move(myDbGrid));

    vDbFilteredGrid* pMyDbGrid = static_cast<vDbFilteredGrid*>(pMainPanel->getChild("myDbGrid"));
    if (pMyDbGrid && pMyDbGrid->getHandle()) {
        pMyDbGrid->populate(L"SELECT * FROM fsna.fsna_2025");
    }
    else {
        ConsoleManager::getInstance().log(L"[ERROR] vDbFilteredGrid 'myDbGrid' este NULL sau nu are un HWND valid.");
    }
    
    // 3. Creează `vDbComboBox`.
    auto myDbComboBox = std::make_unique<vDbComboBox>(
        m_instance,
        "myDbComboBox",
        250, 600, 200, 200, // Am ajustat poziția ca să nu se suprapună
        db
        );
    pMainPanel->addChild("myDbComboBox", std::move(myDbComboBox));

    vDbComboBox* pMyDbComboBox = static_cast<vDbComboBox*>(pMainPanel->getChild("myDbComboBox"));
    if (pMyDbComboBox && pMyDbComboBox->getHandle()) {
        FontManager& fontManager = FontManager::getInstance();
        HFONT customFont = fontManager.getFont(L"Verdana", 14, FW_NORMAL);
        if (customFont) {
            SendMessage(pMyDbComboBox->getHandle(), WM_SETFONT, (WPARAM)customFont, TRUE);
        }
        else {
            ConsoleManager::getInstance().log(L"[ERROR] Nu s-a putut obține fontul pentru ComboBox.");
        }
        pMyDbComboBox->populate(L"SELECT company_name, operator_id FROM date_aviatie.oper LIMIT 10");

        vLabel* pMyLabel = static_cast<vLabel*>(pMainPanel->getChild("statusLabel"));
        if (pMyLabel) {
            pMyDbComboBox->registerHandler("selectionChange", [pMyDbComboBox, pMyLabel]() {
                int selectedIndex = pMyDbComboBox->getSelectedIndex();
                std::wstring selectedText = pMyDbComboBox->getSelectedText();
                LPARAM itemData = pMyDbComboBox->getSelectedItemData();
                pMyLabel->setText(selectedText);
                ConsoleManager::getInstance().log(L"[EVENT] vDbComboBox 'selectionChange': Index=" + std::to_wstring(selectedIndex) +
                    L", Text='" + selectedText + L"', Data=" + std::to_wstring(itemData));
                });
        }
    }

    ConsoleManager::getInstance().log(L"[vDbApp::init] Inițializarea aplicației a fost finalizată.");
    return true;
}
*/
#include "vDbApp.hpp"
#include "ConsoleManager.hpp"
#include "FontManager.hpp"
#include "../odbcConnection.hpp"
#include "vPanel.hpp"
#include "vLabel.hpp"
#include "vDbFilteredGrid.hpp"
#include "vDbComboBox.hpp"

// vApp este clasa de bază. Ea nu ar trebui să conțină logică specifică de UI,
// ci doar scheletul aplicației. Logica UI ar trebui să fie în vDbApp sau într-o clasă derivată.

vDbApp::vDbApp(HINSTANCE hInstance)
    : vApp(hInstance) {
}

void vDbApp::addDbConnection(const std::wstring& name, std::unique_ptr<dbConnection> conn) {
    if (!conn) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbApp::addDbConnection: Încercare de a adăuga o conexiune NULL.");
        return;
    }

    if (!conn->isConnected()) {
        ConsoleManager::getInstance().log(L"[WARNING] vDbApp::addDbConnection: Se adaugă o conexiune neactivă cu numele '" + std::wstring(name.begin(), name.end()) + L"'.");
    }

    m_dbConnections[name] = std::move(conn);
    //ConsoleManager::getInstance().log(L"[LOG] vDbApp::addDbConnection: Conexiunea '" + std::wstring(name.begin(), name.end()) + L"' a fost adăugată.");
}

dbConnection* vDbApp::getDbConnection(const std::wstring& name) {
    auto it = m_dbConnections.find(name);
    if (it != m_dbConnections.end()) {
        return it->second.get();
    }
    ConsoleManager::getInstance().log(L"[WARNING] vDbApp::getDbConnection: Conexiunea '" + std::wstring(name.begin(), name.end()) + L"' nu a fost găsită.");
    return nullptr;
}

bool vDbApp::initGui() {
    /*
    // 1. Inițializează aplicația de bază (fereastra principală, etc.).
    // NOTĂ: Dacă vApp::init() face doar lucuri generice (înregistrarea clasei de fereastră),
    // atunci o poți apela. Dacă face și creare de controale, e mai bine să muți acele
    // apeluri direct în această metodă.
    // Presupunând că vApp::init() doar creează și înregistrează fereastra principală...
    if (!vApp::init()) {
        return false;
    }

    // Obține un pointer la fereastra principală creată de vApp::init().
    vWindow* pMainWindow = getWindow("main");
    if (!pMainWindow) {
        ConsoleManager::getInstance().log(L"[ERROR] Fereastra principală 'main' nu a fost găsită după vApp::init().");
        return false;
    }
    LONG_PTR style = GetWindowLongPtr(pMainWindow->getHandle(), GWL_STYLE);
    style &= ~WS_THICKFRAME;
    // Elimină butoanele de maximizare și minimizare
    style &= ~WS_MAXIMIZEBOX;
    style &= ~WS_MINIMIZEBOX;
    SetWindowLongPtr(pMainWindow->getHandle(), GWL_STYLE, style);

    // 2. Creează panoul principal.
    //auto panel = std::make_unique<vPanel>(m_instance, "mainPanel");
    //pMainWindow->addChild("mainPanel", std::move(panel));
    vPanel* pMainPanel = static_cast<vPanel*>(pMainWindow->getChild("mainPanel"));
    if (!pMainPanel) {
        ConsoleManager::getInstance().log(L"[ERROR] Panoul principal 'mainPanel' nu a fost creat.");
        return false;
    }

    // 3. Inițializează conexiunile la bazele de date
    auto myDb1 = std::make_unique<odbcConnection>("odbc", L"ANC_Server");
    if (myDb1->openDatabase()) {
        this->addDbConnection("ANC_Server", std::move(myDb1));
        ConsoleManager::getInstance().log(L"Baza de date 'ANC_Server' a fost deschisă și adăugată.");
    }
    else {
        ConsoleManager::getInstance().log(L"[ERROR] Nu s-a putut deschide baza de date 'ANC_Server'.");
    }

    // Poți inițializa alte conexiuni aici...
    // auto myDb2 = ...

    dbConnection* db = getDbConnection("ANC_Server");
    if (db && db->isConnected()) {
        // Creezi direct un obiect de tip vDbFilteredGrid.
        auto myDbGrid = std::make_unique<vDbFilteredGrid>(
            m_instance,
            "myDbGrid",
            40, 250, 700, 250,
            //0, 0, 780, 580,
            db // Aici folosești pointerul brut valid
            );
        pMainPanel->addChild("myDbGrid", std::move(myDbGrid));

        // Aici, cast-ul este de asemenea important.
        // Trebuie să convertești la tipul corect.
        vDbFilteredGrid* pMyDbGrid = static_cast<vDbFilteredGrid*>(pMainPanel->getChild("myDbGrid"));
        if (pMyDbGrid && pMyDbGrid->getHandle()) {
          //  pMyDbGrid->createControls();
            pMyDbGrid->populate(L"SELECT TRIM(nr_fsna) AS nr_fsna, TRIM(arcid_dep) AS arcid_dep, reg AS Registration FROM fsna.fsna_2025");
        }
        else {
            ConsoleManager::getInstance().log(L"[ERROR] vDbFilteredGrid 'myDbGrid' este NULL sau nu are un HWND valid.");
        }
    }
    else {
        ConsoleManager::getInstance().log(L"[WARNING] Nu se poate crea vDbGrid-ul; conexiunea la baza de date este invalidă.");
    }

    // Aici adaugi restul codului pentru a crea butoane, combo box-uri etc.
    // Toate trebuie să fie sub controlul `pMainPanel`.
    // ... adaugă restul codului de inițializare a UI-ului de aici ...

    // În vDbApp::init()

// Obține conexiunea la baza de date.
        if (db && db->isConnected()) {
            // Creează un vDbComboBox
            auto myDbComboBox = std::make_unique<vDbComboBox>(
                m_instance,
                "myDbComboBox",
                250, 100, 200, 200,
                db // Aici se pasează pointerul la baza de date
                );

            // Adaugă ComboBox-ul la panou.
            pMainPanel->addChild("myDbComboBox", std::move(myDbComboBox));

            // Obține un pointer valid și populează-l.
            vDbComboBox* pMyDbComboBox = static_cast<vDbComboBox*>(pMainPanel->getChild("myDbComboBox"));
            if (pMyDbComboBox && pMyDbComboBox->getHandle()) {
                // Query-ul selectează numele operatorilor și un ID asociat.
                // Obține instanța FontManager-ului
                FontManager& fontManager = FontManager::getInstance();

                // Obține HFONT-ul dorit de la FontManager
                HFONT customFont = fontManager.getFont(L"Verdana", 14, FW_NORMAL);

                if (customFont) {
                    // Trimite mesajul WM_SETFONT către controlul ComboBox
                    // Parametrul TRUE forțează controlul să se re-deseneze imediat
                    SendMessage(pMyDbComboBox->getHandle(), WM_SETFONT, (WPARAM)customFont, TRUE);
                }
                else {
                    // Loghează o eroare dacă fontul nu a putut fi creat
                    ConsoleManager::getInstance().log(L"[ERROR] Nu s-a putut obține fontul pentru ComboBox.");
                }

                pMyDbComboBox->populate(L"SELECT company_name, operator_id FROM date_aviatie.oper LIMIT 10");
            }

            vLabel* pMyLabel = static_cast<vLabel*>(pMainPanel->getChild("statusLabel"));
            pMyDbComboBox->registerHandler("selectionChange", [pMyDbComboBox,pMyLabel]() {
                // Poți captura și alte controale dacă ai nevoie, la fel ca în exemplul anterior
                // de ex: [pMyDbComboBox, pMyLabel]()

                // Obține indexul selectat
                int selectedIndex = pMyDbComboBox->getSelectedIndex();
                // Obține textul selectat
                std::wstring selectedText = pMyDbComboBox->getSelectedText();
                // Obține datele asociate (LPARAM)
                LPARAM itemData = pMyDbComboBox->getSelectedItemData();
                pMyLabel->setText(selectedText);
                // Loghează informațiile
                ConsoleManager::getInstance().log(L"[EVENT] vDbComboBox 'selectionChange': Index=" + std::to_wstring(selectedIndex) +
                    L", Text='" + selectedText + L"', Data=" + std::to_wstring(itemData));

                // Aici poți adăuga logica ta personalizată, de exemplu:
                // - Populezi o altă grilă sau un alt ComboBox pe baza `itemData`
                // - Afișezi informații suplimentare într-un Label sau EditBox
                });
        }


    ConsoleManager::getInstance().log(L"[vDbApp::init] Inițializarea aplicației a fost finalizată.");
    */
    return true;
}
