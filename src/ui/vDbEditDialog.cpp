#include "vApp.hpp"
#include "vButton.hpp"
#include "vSpacer.hpp"
#include "vDbFormPanel.hpp"
#include "vDbEditDialog.hpp"
#include "vMessageDialog.hpp"
#include "Layouts/Layouts.hpp"


vDbEditDialog::vDbEditDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher, dbConnection* db, EditMode mode) :
    vWindow(hInstance, id, WindowType::DialogWindow, false, dispatcher),
    m_db(db),
    m_mode(mode)
{
    // 1. Root Layout
    this->setLayoutStrategy(std::make_unique<VerticalStackLayout>());

    // Creăm fereastra fizică
    this->create(L"VDbGridEditClass", L"Edit record", WS_OVERLAPPEDWINDOW,
        100, 100, 600, 500,
        vApp::getAppInstance()->getMainWindow(), nullptr);
    // Verificăm dacă avem HWND valid, la fel ca în Grid
    if (getHandle()) {
        
        // 2. Panel Formular (Zonă FILL)
        auto editPanel = std::make_unique<vDbFormPanel>(m_hInstance, "editPanel", 0, 0, 600, 350, getEventDispatcher());
        editPanel->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        editPanel->setHeightMode(SizeMode::FILL);
        editPanel->setWidthMode(SizeMode::FILL);
        editPanel->setMargins(10, 10, 10, 10);

        m_DbFormPanel = editPanel.get();
        
        //setupEditControls();

        this->addChild("editPanel", std::move(editPanel));
        

        const int topH = 30;    // în loc de MulDiv(20, ...)
        const int filterH = 28; // 12 era prea mic pentru un Edit Box
        const int bottomH = 32;
        const int statusH = 24;

        // 3. Panel Butoane (Zonă FIXED - ca în setupActionControls)
        auto actPanel = std::make_unique<vPanel>(m_hInstance, "actionPanel", 0, 0, m_width, topH, getEventDispatcher());
        actPanel->setHeightMode(SizeMode::FIXED);
        actPanel->setWidthMode(SizeMode::FILL);
        //topPanel->setBackgroundColor(RGB(255, 0, 0));
        actPanel->setLayoutStrategy(std::make_unique<FlexStackLayout>());
        actPanel->setMargins(10, 10, 10, 10);
        m_actionPanel = actPanel.get();
        this->addChild("topPanel", std::move(actPanel));

        // Luăm valorile de succes din grid-ul tău
        const int btnW = 100;
        const int btnH = 30;
        const int btnY = (m_actionPanel->getHeight() - btnH) / 2;

        auto leftSpring = std::make_unique<vSpacer>("spacer_L", 0, btnY, 0, btnW, getEventDispatcher());
        leftSpring->setWidthMode(SizeMode::FILL); // Ocupă tot spațiul liber din stânga
        m_actionPanel->addChild("spacer_L", std::move(leftSpring));

        auto actButton = std::make_unique<vButton>(m_hInstance, "actBut", L"Actiune", 10, btnY, btnW, btnH, getEventDispatcher());
        // Adăugăm butoanele direct (ele vor fi repoziționate de FlexStackLayout)
        /*
        if (m_mode == EditMode::Update) {
            //m_actionPanel->addChild("btnSave", std::make_unique<vButton>(m_hInstance, "btnSave", L"Salvează", 10, btnY, btnW, btnH, getEventDispatcher()));
            actButton->setText(L"Salvează");
            actButton->setId("btnSave");
            getEventDispatcher().registerHandler("click", "btnSave", [this]() { this->executeAction();  });
        }
        else if (m_mode == EditMode::Insert) {
            actButton->setText(L"Adaugă");
            actButton->setId("btnAdd");
            //m_actionPanel->addChild("btnAdd", std::make_unique<vButton>(m_hInstance, "btnAdd", L"Adaugă", 10, btnY, btnW, btnH, getEventDispatcher()));
            getEventDispatcher().registerHandler("click", "btnAdd", [this]() { this->executeAction();  });
        }
        else if (m_mode == EditMode::Delete) {
            actButton->setText(L"Şterge");
            actButton->setId("btnDel");
            //m_actionPanel->addChild("btnDel", std::make_unique<vButton>(m_hInstance, "btnDel", L"Şterge", 10, btnY, btnW, btnH, getEventDispatcher()));
            getEventDispatcher().registerHandler("click", "btnDel", [this]() { this->executeAction();  });
        }
        */
        m_actionButton = actButton.get();
        m_actionPanel->addChild(actButton->getId(), std::move(actButton));

        m_actionPanel->addChild("btnCancel", std::make_unique<vButton>(m_hInstance, "btnCancel", L"Renunță", 10, btnY, btnW, btnH, getEventDispatcher()));

        auto rightSpring = std::make_unique<vSpacer>("spacer_R", 0, 0, 0, 0, getEventDispatcher());
        rightSpring->setWidthMode(SizeMode::FILL); // Ocupă tot spațiul liber din dreapta
        m_actionPanel->addChild("spacer_R", std::move(rightSpring));
        m_actionPanel->applyLayout();

        // 4. Handlere
        getEventDispatcher().registerHandler("click", "btnCancel", [this]() { this->hide(); });
        // 5. TRUCUL DIN GRID: Forțăm un refresh de layout
        this->applyLayout();

        // Trimitem mesajul de resize pentru a forța Windows să recalculeze ierarhia
        //PostMessage(getHandle(), WM_SIZE, 0, 0);
        //SetWindowLongPtr(getHandle(), GWL_HWNDPARENT, (LONG_PTR)vApp::getAppInstance()->getMainWindow());
    }
}


void vDbEditDialog::updateUIForMode() {

    setupActionButtons();

    switch (m_mode) {
    case EditMode::Insert:
        SetWindowText(getHandle(), L"Adăugare Înregistrare Nouă");
        // Aici poți activa toate câmpurile
        break;
    case EditMode::Update: {

        SetWindowText(getHandle(), std::wstring(L"Editare Înregistrare:" + str_to_wstr(m_uniqueIdField) + L":" + m_uniqueIdValue).c_str());
        break; 
    }
    case EditMode::Delete:
        SetWindowText(getHandle(), L"Confirmare Ștergere");
        m_DbFormPanel->setAllControlsEnabled(false);
        // Aici poți face toate câmpurile ReadOnly
        break;
    }
}

// Această metodă va fi apelată de butonul "OK/Salvează"
void vDbEditDialog::executeAction() {
    bool ret = false;
    LOG_DEBUG(L"EXECUT ACTIUNE - Mod: " + std::to_wstring((int)m_mode));

    if (m_mode == EditMode::Delete) {
        ret = deleteRecord(); // Dacă userul apasă NO, ret va fi FALSE
    }
    // Adaugă aici ramurile pentru Insert/Update când sunt gata, setând ret = true doar la succes

    if (ret) {
        LOG_DEBUG(L"Acțiune confirmată. Închid fereastra de editare.");
        m_actionSuccessful = true;
        this->hide();
    }
    else {
        LOG_INFO(L"Acțiune infirmată. Fereastra rămâne deschisă.");
    }
}



void vDbEditDialog::setupActionButtons() {
    std::string newId;
    std::wstring newText;

    // 1. Stabilim datele în funcție de mod
    switch (m_mode) {
    case EditMode::Update: newId = "btnSave"; newText = L"Salvează"; break;
    case EditMode::Insert: newId = "btnAdd";  newText = L"Adaugă";   break;
    case EditMode::Delete: newId = "btnDel";  newText = L"Șterge";    break;
    }

    // 2. Curățăm și actualizăm
    getEventDispatcher().removeHandlers(m_actionButton->getId());
    m_actionButton->setText(newText);

    if (m_actionButton->setId(newId)) {
        getEventDispatcher().registerHandler("click", newId, [this]() {
            this->executeAction();
            //this->hide();
            });
    }
}


void vDbEditDialog::loadData(const std::map<std::wstring, std::wstring>& rowData) {
    if (!m_DbFormPanel) return;
    
    for (auto& field : m_DbFormPanel->getFormControls()) {
        // Convertim field.dbField (std::string) la std::wstring pentru căutare
        std::wstring wFieldName = str_to_wstr(field.dbField);
        //LOG_ERROR(L"POPULEZ:"+ str_to_wstr(field.uiControl->getId()));
        // Verificăm dacă există cheia în map și dacă avem controlul creat
        if (field.uiControl && rowData.count(wFieldName)) {
            field.uiControl->setText(rowData.at(wFieldName));
        }
    }
}

bool vDbEditDialog::populateControls() {
    // 1. Verificări preliminare
    if (m_mode == EditMode::Insert) return true;
    if (!m_db || !m_DbFormPanel || !hasValidIdentity()) return false;

    // 2. Construim query-ul (folosind sub-query pentru siguranță)
    //std::wstring selectQuery = L"SELECT * FROM ( " + m_query + L" ) AS subq WHERE " +
    std::wstring selectQuery = L"SELECT * FROM " + m_editTable + L" WHERE " +
        str_to_wstr(m_uniqueIdField) + L" = '" + m_uniqueIdValue + L"'";
    m_query = selectQuery;
    // 3. Executăm query-ul
    std::string stm_name = m_id + "_populate"; // ID unic pentru statement
    if (!m_db->execQuery(selectQuery, stm_name)) {
        LOG_ERROR(L"vDbEditDialog::populateControls: Query failed.");
        LOG_DEBUG(selectQuery);

        return false;
    }


    if(m_db->fetchNextRow(stm_name)) {
        auto results = m_db->fetchMap(stm_name);
      
        if (!results.empty()) {
            this->loadData(results);
            LOG_INFO(L"vDbEditDialog: Controale populate cu succes pentru ID: " + m_uniqueIdValue);
            return true;
        }
        else {
            LOG_ERROR(L"vDbEditDialog: Nu s-au găsit date pentru " + str_to_wstr(m_uniqueIdField) + L" = " + m_uniqueIdValue);
            return false;
        }
    }
   
}

bool vDbEditDialog::deleteRecord() {
    LOG_DEBUG(L"SUNT IN deleteRecord - Folosesc dialogul custom");

    // Apelăm metoda statică a clasei vMessageDialog
    // Parametri: hInstance, dispatcher, titlu, mesaj, butoane
    std::string result = vMessageDialog::show(
        L"Confirmare Ștergere",
        L"Ești sigur că vrei să ștergi această înregistrare?",
        MessageButtons::YesNo
    );

    if (result == "yes") {
        LOG_DEBUG(L"Utilizatorul a ales DA (Dialog Custom)");

        // Aici pui logica de SQL Delete:
        // m_db->execQuery(L"DELETE FROM ...", "delete_tag");

        return true;
    }

    LOG_DEBUG(L"Utilizatorul a ales NU sau a închis dialogul.");
    return false;
}