#include "vDbGridPicker.hpp"
#include "vWindow.hpp"
#include "vApp.hpp"
#include "vDbGrid.hpp"
#include "vSpacer.hpp"
#include "vSeparator.hpp"
#include "vMessageDialog.hpp"
#include "Layouts/Layouts.hpp"

void vDbGridPicker::create(HWND parent)  {
    vPanel::create(parent);
    if (!m_handle) return;

    // Dezactivăm orice strategie de layout automată pentru a prelua controlul manual
    this->setLayoutStrategy(nullptr);

    int btnW = 35;
    int spacing = 2;
    // Calculăm lățimea inițială bazată pe m_width-ul actual al panelului
    int editW = (m_width > btnW + spacing) ? (m_width - btnW - spacing) : 0;

    // Creăm edit-ul
    auto edit = std::make_unique<vEdit>(m_hInstance, m_id + "_edit", 0, 0, editW, m_height, m_dispatcher);
    m_edit = edit.get();
    m_edit->setBackgroundColor(RGB(255, 255, 255));

    // Creăm butonul la capătul edit-ului
    auto btn = std::make_unique<vButton>(m_hInstance, m_id + "_btn", L"...", editW + spacing, 0, btnW, m_height, m_dispatcher);
    m_btnOpen = btn.get();

    // Propagăm stilurile (font, culori)
    if (m_hasCustomTextColor) {
        m_edit->setTextColor(m_textColor);
        m_btnOpen->setTextColor(m_textColor);
    }

    this->addChild(m_id + "_edit", std::move(edit));
    this->addChild(m_id + "_btn", std::move(btn));

    m_btnOpen->on("click", [this]() { this->openGridSelector(); });

    // Forțăm primul calcul de dimensiuni
    this->resize();
}


void vDbGridPicker::resize() {
    // 1. ÎNTAI actualizăm m_width și m_height din dimensiunile REALE ale ferestrei (HWND)
    if (m_handle) {
        RECT rc;
        GetClientRect(m_handle, &rc);
        m_width = rc.right - rc.left;
        m_height = rc.bottom - rc.top;
    }

    // 2. LOG: Acum Width ar trebui să fie 1444, nu 200
    //LOG_ERROR(L"[Picker Log] ID: " + str_to_wstr(m_id) +
    //    L" | Width Real: " + std::to_wstring(m_width));

    if (m_edit && m_btnOpen) {
        int btnW = 35;
        int spacing = 2;
        int editW = m_width - btnW - spacing;

        if (editW < 0) editW = 0;

        // 3. LOG: Vedem dacă acum setăm peste 1400px
       // LOG_ERROR(L"  -> Setare Edit Width: " + std::to_wstring(editW));

        // 4. Aplicăm dimensiunile copiilor
        m_edit->moveAndResize(0, 0, editW, m_height);
        m_btnOpen->moveAndResize(editW + spacing, 0, btnW, m_height);
    }
}

void vDbGridPicker::openGridSelector() {
    vApp* app = vApp::getAppInstance();
    if (!app) return;

    std::string winId = m_id + "_GridWin";
    vWindow* existingWin = app->getWindow(winId);

    // 1. Verificare existență fereastră
    if (existingWin && IsWindow(existingWin->getHandle())) {
        ShowWindow(existingWin->getHandle(), SW_RESTORE);
        SetForegroundWindow(existingWin->getHandle());
        return;
    }
    else if (existingWin) {
        app->removeWindow(winId);
    }

    // 2. Creare fereastră Dialog
    auto gridWin = std::make_unique<vWindow>(app->getInstance(), winId, WindowType::DialogWindow, false, getEventDispatcher());
    gridWin->create(L"VDbPicker_" + str_to_wstr(m_id), L"Selectați înregistrarea",
        WS_OVERLAPPEDWINDOW, 200, 200, 900, 600, m_handle, nullptr);

    gridWin->setLayoutStrategy(std::make_unique<AnchorLayout>());

    // 3. Grid-ul (Zonă Centrală)
    auto grid = std::make_unique<vDbFilteredGrid>(app->getInstance(), "grid_" + m_id, 0, 0, 100, 100, getEventDispatcher(), m_db);
    grid->setAnchor(Anchor::LEFT | Anchor::TOP | Anchor::RIGHT | Anchor::BOTTOM);
    grid->setHeightMode(SizeMode::FILL);
    grid->setWidthMode(SizeMode::FILL);
    // Margin Bottom mare (80) ca să nu acopere butoanele de paginare proprii și panelul nostru
    grid->setMargins(10, 10, 10, 80);
    grid->setEditable(false);

    vDbFilteredGrid* pGridRaw = grid.get();
    gridWin->addChild("grid_ctrl", std::move(grid));

   
   
    // 4. Panel-ul de Butoane (Zonă Inferioară)
    // Îi dăm o înălțime fixă de 50px
    auto bottomPanel = std::make_unique<vPanel>(app->getInstance(), "bottom_bar", 0, 0, 100, 50, getEventDispatcher());
    bottomPanel->setAnchor(Anchor::LEFT | Anchor::RIGHT | Anchor::BOTTOM);
    bottomPanel->setWidthMode(SizeMode::FILL);
    bottomPanel->setMargins(10, 0, 10, 10); // 10px distanță de marginea de jos
    bottomPanel->setLayoutStrategy(std::make_unique<FlexStackLayout>());

    vPanel* pBottom = bottomPanel.get();
    gridWin->addChild("bottom_bar", std::move(bottomPanel));



   
    // 5. Adăugare elemente în Panelul de jos (FLEX)

    // Spacer-ul împinge totul la dreapta
    auto spacer = std::make_unique<vSpacer>("bottom_spacer", 0, 0, 0, 0, getEventDispatcher());
    spacer->setWidthMode(SizeMode::FILL);
    pBottom->addChild("spacer", std::move(spacer));

    // Buton Alege
    auto btnSelect = std::make_unique<vButton>(app->getInstance(), "btn_select", L"Alege", 0, 0, 120, 35, getEventDispatcher());
    btnSelect->setBackgroundColor(RGB(0, 120, 215));
    btnSelect->setTextColor(RGB(255, 255, 255));
    

        // Definim o funcție lambda reutilizabilă pentru a evita duplicarea codului
    /*
    auto finalizeSelection = [this, pGridRaw, winId, app](int rowIndex) {
        vDbGrid* internalGrid = pGridRaw->getDbGrid();
        if (!internalGrid || rowIndex < 0) return;

        // Extragem valoarea folosind field name-ul dorit
        std::wstring idVal = trim(internalGrid->getCellValueByFieldName(rowIndex, wstr_to_str(m_returnIdColumn)));

        std::wstring valoare = trim(internalGrid->getCellValueByFieldName(rowIndex, wstr_to_str(m_returnColumn)));

        // Actualizăm picker-ul și închidem
        this->setSelectedValue(idVal);  //
        this->setText(valoare);
        this->getEventDispatcher().dispatch("item_selected", m_id, wstr_to_str(valoare));
        app->removeWindow(winId);
    };
    */

    auto finalizeSelection = [this, pGridRaw, winId, app](int rowIndex) {
        vDbGrid* internalGrid = pGridRaw->getDbGrid();
        if (!internalGrid || rowIndex < 0) return;

        // 1. Curățăm datele vechi
        m_selectedRowData.clear();

        // 2. Extragem absolut TOATE coloanele din rândul selectat
        int colCount = internalGrid->getColumnCount();
        for (int i = 0; i < colCount; ++i) {
            std::wstring colName = internalGrid->getColumnFieldName(i);
            std::wstring colValue = internalGrid->getCellValue(rowIndex, i);

            // Salvăm în map: NumeColoană -> Valoare
            m_selectedRowData[colName] = trim(colValue);
        }

        // 3. Setăm valorile pentru afișarea în Edit-ul Picker-ului
        std::wstring idVal = trim(internalGrid->getCellValueByFieldName(rowIndex, wstr_to_str(m_returnIdColumn)));
        std::wstring valoare = trim(internalGrid->getCellValueByFieldName(rowIndex, wstr_to_str(m_returnColumn)));

        this->setSelectedValue(idVal);
        this->setText(valoare);

        // 4. Notificăm sistemul (acum map-ul m_selectedRowData este plin și poate fi citit)
        this->getEventDispatcher().dispatch("item_selected", m_id, wstr_to_str(valoare));

        app->removeWindow(winId);
    };

    // A) Logică pentru butonul "Alege"
    btnSelect->on("click", [finalizeSelection, pGridRaw]() {
        int selectedRow = pGridRaw->getDbGrid()->getSelectedRow();

        if (selectedRow >= 0) {
            // Dacă avem selecție, finalizăm
            finalizeSelection(selectedRow);
        }
        else {
            // Dacă NU avem selecție, afișăm mesajul de eroare
            vMessageDialog::Warning(
                L"Vă rugăm să selectați o înregistrare din listă înainte de a apăsa butonul Alege.",
                L"Selecție necesară"
            );
        }
        });

    // B) Logică pentru Dublu-Click pe rând
    pGridRaw->setOnCellDblClick([finalizeSelection](int row, int col, const std::wstring& content) {
        finalizeSelection(row);
        });

    pBottom->addChild("btn_select", std::move(btnSelect));

    // Buton Închide
    auto btnCancel = std::make_unique<vButton>(app->getInstance(), "btn_cancel", L"Închide", 0, 0, 120, 35, getEventDispatcher());
    btnCancel->setMargins(10, 0, 0, 0); // Spațiu între butoane
    btnCancel->on("click", [winId, app]() {
        app->removeWindow(winId);
        });
    pBottom->addChild("btn_cancel", std::move(btnCancel));


    // 6. Afișare
    
    pGridRaw->populate(m_targetQuery);
    gridWin->applyLayout();

    vWindow* pWin = gridWin.get();
    app->addWindow(winId, std::move(gridWin));
    pWin->show();
}


LRESULT vDbGridPicker::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SIZE) {
        // Când bunicul/părintele ne schimbă dimensiunea (ex: la 1444px)
        // LOWORD(lParam) este lățimea nouă, HIWORD(lParam) este înălțimea
        int newW = LOWORD(lParam);
        int newH = HIWORD(lParam);

        // Actualizăm membrii interni ca să fim siguri
        m_width = newW;
        m_height = newH;

        // Re-aliniem copiii imediat
        if (m_edit && m_btnOpen) {
            int btnW = 35;
            int spacing = 2;
            int editW = newW - btnW - spacing;
            if (editW < 0) editW = 0;

            m_edit->moveAndResize(0, 0, editW, newH);
            m_btnOpen->moveAndResize(editW + spacing, 0, btnW, newH);
        }
    }
    return vPanel::handleMessage(hwnd, msg, wParam, lParam);
}