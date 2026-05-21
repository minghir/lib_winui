#include "vDbFilteredGrid.hpp"
#include "vButton.hpp"
#include "vDbGrid.hpp"
#include "ConsoleManager.hpp"
#include "ControlIdManager.hpp"
#include "FontManager.hpp"
#include "vSpacer.hpp"
#include "vPopupMenu.hpp"
#include "vStatusBar.hpp"
#include "vWindow.hpp"
#include "vWaitCursor.hpp"
#include "vApp.hpp"
#include "vMessageDialog.hpp"


#include "../sql/SqlQueryAnalyzer.hpp"
#include "../stringUtils.hpp"
#include "ILayoutStrategy.hpp"
#include "Layouts\Layouts.hpp"


#include "vSaveFileDialog.hpp"
#include <fstream>
#include <thread>
#include <algorithm>
#include <filesystem>

// Constructor
vDbFilteredGrid::vDbFilteredGrid(
    HINSTANCE hInstance,
    const std::string& id,
    int x, int y, int width, int height,
    EventDispatcher& dispatcher,
    dbConnection* db
    //,const std::map<std::wstring, std::wstring>& columnLabels
) : vPanel(hInstance, id, x, y, width, height, dispatcher),
//m_columnLabels(columnLabels),
m_db(db),
m_grid(nullptr)
{
    //setBackgroundColor(RGB(224, 224, 0));
    setLayoutStrategy(std::make_unique<FlexStackLayout>());
   // ConsoleManager::getInstance().log(L"[vDbFilteredGrid::Constructor] Apelat pentru ID: " + str_to_wstr(id));
}

// --- Metoda `create` ---
void vDbFilteredGrid::create(HWND parent) {
    vPanel::create(parent);

    if (m_handle) {
      //  ConsoleManager::getInstance().log(L"[vDbFilteredGrid::create] Creare controale copil pe un HWND valid.");
        createControls(m_handle);
        //createControls(parent);
        // Trimitem un mesaj către noi înșine să facem resize imediat după ce se termină crearea
        PostMessage(m_handle, WM_SIZE, 0, 0);
    }
    else {
        LOG_ERROR(L"vDbFilteredGrid::create: Nu s - a putut crea propriul HWND.Controalele copil nu vor fi create.");
    }

}


void vDbFilteredGrid::createControls(HWND parent) {
    //vSpacer::s_debugMode = true;

    
    // 1. Inițializăm containerul principal
    RECT rc;
    GetClientRect(parent, &rc);
    m_width = rc.right - rc.left;
    m_height = rc.bottom - rc.top;
    scale(GetDpiForWindow(parent));

    // 2. SETĂM STRATEGIA DE LAYOUT
    this->setLayoutStrategy(std::make_unique<VerticalStackLayout>());

    const int topH = 30;    // în loc de MulDiv(20, ...)
    const int filterH = 28; // 12 era prea mic pentru un Edit Box
    const int bottomH = 32;
    const int statusH = 24;

    // 3. ADĂUGĂM CONTROALELE (Ordinea adăugării contează pentru VerticalStack!)

    // A. TOP PANEL (Fixed)
    if (m_hasActionBar) {
        auto topPanel = std::make_unique<vPanel>(m_hInstance, "topPanel", 0, 0, m_width, topH, getEventDispatcher());
        topPanel->setHeightMode(SizeMode::FIXED);
        topPanel->setWidthMode(SizeMode::FILL);
        topPanel->setBackgroundColor(RGB(255, 255, 255));
        topPanel->setLayoutStrategy(std::make_unique<FlexStackLayout>());
        topPanel->setMargins(0, 0, 0, 0);
        m_topPanel = topPanel.get();
        addChild("topPanel", std::move(topPanel));
        setupActionControls();
    }

    // B. FILTRE (Fixed)
    if (m_isFilterable) {
        auto filterPanel = std::make_unique<vPanel>(m_hInstance, "filterPanel", 0, 0, m_width, filterH, getEventDispatcher());
        filterPanel->setHeightMode(SizeMode::FIXED);
        filterPanel->setWidthMode(SizeMode::FILL);
        filterPanel->setBackgroundColor(RGB(255, 255, 255));
        //filterPanel->setBackgroundColor(RGB(0, 125, 0));
        filterPanel->setMargins(0, 0, 0, 0);
        m_filterPanel = filterPanel.get();
        addChild("filterPanel", std::move(filterPanel));
    }

    // C. GRID (FILL - acesta va ocupa tot spațiul central!)
    //std::string gridId = m_id + "_mainGrid";
    //auto grid = std::make_unique<vDbGrid>(m_hInstance, gridId, 0, 0, m_width, 0, getEventDispatcher(), m_db);
    auto grid = std::make_unique<vDbGrid>(m_hInstance, m_id, 0, 0, m_width, 0, getEventDispatcher(), m_db);

    grid->setHeightMode(SizeMode::FILL); // <--- IMPORTANT
    grid->setWidthMode(SizeMode::FILL);
    grid->setMargins(0, 0, 0, 0);
    m_grid = grid.get();

    //addChild(gridId, std::move(grid));
    addChild(m_id + "_internal", std::move(grid)); // Folosim un nume intern doar pentru map-ul de copii

    getEventDispatcher().registerHandler("grid_row_dblclick", m_id + "_internal", [this](const std::string& arg) {
        std::vector<std::string> tokens = explode(arg, ';');
        if (tokens.size() >= 3) {
            int r = std::stoi(tokens[0]);
            int c = std::stoi(tokens[1]);
            std::wstring text = utf8_to_wstring(tokens[2]);

            // Apelăm metoda dorită de tine
            this->onCellDblClick(r, c, text);
        }
        });




    // D. BOTTOM PANEL (Fixed)
    if (m_isPaginable) {
        auto bottomPanel = std::make_unique<vPanel>(m_hInstance, "bottomPanel", 0, 0, m_width, bottomH, getEventDispatcher());
        bottomPanel->setHeightMode(SizeMode::FIXED);
        bottomPanel->setWidthMode(SizeMode::FILL);
        
        bottomPanel->setBackgroundColor(RGB(255, 255,255));
        bottomPanel->setLayoutStrategy(std::make_unique<FlexStackLayout>());
        bottomPanel->setMargins(0, 0, 0, 0);
        m_bottomPanel = bottomPanel.get();
        addChild("bottomPanel", std::move(bottomPanel));

        setupPagingControls(); // Butoanele se vor așeza în interiorul lui bottomPanel
    }

    // E. STATUS BAR (Fixed)
    if (m_hasStatusBar) {
        auto statusBar = std::make_unique<vStatusBar>("mainStatusBar", 0, 0, m_width, statusH, getEventDispatcher());
        statusBar->setHeightMode(SizeMode::FIXED);
        statusBar->setWidthMode(SizeMode::FILL);
        statusBar->setMargins(0, 0, 0, 0);
        m_statusBar = statusBar.get();
        addChild("mainStatusBar", std::move(statusBar));
    }

    // 4. EXECUTĂM LAYOUT-UL
    this->applyLayout();
}

/*
void vDbFilteredGrid::resize() {

    RECT rc;
    GetClientRect(m_handle, &rc);
    LOG_DEBUG(L"[vDbFilteredGrid] CLIENT SIZE: " +
        std::to_wstring(rc.right) + L"x" +
        std::to_wstring(rc.bottom));

    if (m_grid) {
        RECT rcGrid;
        GetWindowRect(m_grid->getHandle(), &rcGrid);
        LOG_DEBUG(L"[vDbFilteredGrid] GRID SIZE IN RESIZE: " +
            std::to_wstring(rcGrid.right - rcGrid.left) + L"x" +
            std::to_wstring(rcGrid.bottom - rcGrid.top));
    }



    // 1. Recalculăm pozițiile automate conform VerticalStackLayout
    //this->applyLayout();

    // 2. Singurul lucru manual: Sincronizarea filtrelor cu coloanele gridului
    // (Asta nu o poate face VerticalStack pentru că e logică internă de business)
    if (m_grid && m_filterPanel) {
        int scrollX = GetScrollPos(m_grid->getHandle(), SB_HORZ);
        int currentX = 0;

        for (size_t i = 0; i < m_filterEdits.size(); ++i) {
            int colW = m_grid->getColumnWidthByIndex(i);
            m_filterEdits[i]->moveAndResize(currentX - scrollX, 0, colW, m_filterPanel->getHeight());
            currentX += colW;
        }
    }

    
}
*/
/*
void vDbFilteredGrid::resize() {
    // 1. Obținem DPI-ul curent (poate s-a schimbat)
    int currentDpi = GetDpiForWindow(m_handle);

    // 2. IMPORTANT: Spune-i grid-ului să se scalaze (asta va apela vGrid::scale -> vGrid::resize)
    if (m_grid) {
        m_grid->scale(currentDpi);
    }

    // 3. Restul logicii tale pentru filtre
    if (m_grid && m_filterPanel) {
        int scrollX = GetScrollPos(m_grid->getHandle(), SB_HORZ);
        int currentX = 0;
        for (size_t i = 0; i < m_filterEdits.size(); ++i) {
            int colW = m_grid->getColumnWidthByIndex(i);
            m_filterEdits[i]->moveAndResize(currentX - scrollX, 0, colW, m_filterPanel->getHeight());
            currentX += colW;
        }
    }
}
*/
void vDbFilteredGrid::resize() {
    // 1. Întâi lăsăm layout-ul să așeze grid-ul în panel
    vPanel::resize();

    // 2. Acum grid-ul are dimensiunea corectă, îi spunem să-și scaleze coloanele interne
    if (m_grid) {
        m_grid->resize();
    }

    // 3. Aliniem filtrele de deasupra grid-ului
    syncFilterScroll(); // Metoda ta care mută edit-urile conform coloanelor
}

void vDbFilteredGrid::setTitle(const std::wstring title) { 
    m_title = title; 
    if (m_titleLabel) {
        m_titleLabel->setText(title);
        //m_titleLabel->setFontSize(int(m_titleLabel->getFontSize() * 1.4));
        m_titleLabel->setFontWeight(FW_BOLD);
    }
    else {
        // Dacă nu există încă, titlul va fi afișat oricum 
        // la următoarea populare a setupPagingControls 
        // deoarece am salvat valoarea în m_title.
        LOG_WARNING(L"setTitle: m_titleLabel nu este încă inițializat.");
    }
}

void vDbFilteredGrid::setupPagingControls() {
    if (!m_bottomPanel ) return;
    if (!m_isPaginable) return; // Mută verificarea la început!
    // 1. Configurare dimensiuni (păstrăm valorile tale)
    const int btnW = 28;
    const int spacing = 4;
    const int labelWidth = 350;
    const int btnY = (m_bottomPanel->getHeight() - btnW) / 2;

    // 2. Adăugăm un "spring" (spacer) dacă vrei ca butoanele să stea în dreapta
    // Dacă vrei butoanele în stânga, șterge această linie
   /*
    auto titleLabel = std::make_unique<vLabel>(m_hInstance, "titleLabel", m_title, 0, btnY, labelWidth, btnW, getEventDispatcher());
    m_titleLabel = titleLabel.get();
    m_bottomPanel->addChild("titleLabel", std::move(titleLabel));
    */


    m_bottomPanel->addChild("spring", std::make_unique<vSpacer>("spring", 0, btnY, 10, btnW, getEventDispatcher()));

   

    // 3. Creare butoane de navigare
    // NOTĂ: Folosim addChild returnând pointerul direct pentru a salva referințele în m_firstBtn, m_prevBtn, etc.
    // 
    // 4. Label-ul pentru textul paginării (Ex: "Pagina 1 din 10")

    

    auto paginationLabel = std::make_unique<vLabel>(m_hInstance, "paginationLabel", L"", 0, btnY, labelWidth, btnW, getEventDispatcher());
    m_paginationLabel = paginationLabel.get();
    m_bottomPanel->addChild("paginationLabel", std::move(paginationLabel));


    auto makeId = [this](const std::string& subId) { return m_id + "_" + subId; };

    // Creare componente cu ID-uri prefixate
    std::string firstId = makeId("firstPageBtn");
    std::string prevId = makeId("prevPageBtn");
    std::string nextId = makeId("nextPageBtn");
    std::string lastId = makeId("lastPageBtn");


    // First Page
    auto firstBtn = std::make_unique<vButton>(m_hInstance, firstId, L"|\u25C0", 0, btnY, btnW, btnW, getEventDispatcher());
    m_firstBtn = firstBtn.get();
    m_bottomPanel->addChild(firstId, std::move(firstBtn));
    //m_firstBtn->setFont(L"Arial", 8);

    // Prev Page
    auto prevBtn = std::make_unique<vButton>(m_hInstance, prevId, L"\u25C0", 0, btnY, btnW, btnW, getEventDispatcher());
    m_prevBtn = prevBtn.get();
    m_bottomPanel->addChild(prevId, std::move(prevBtn));
    //m_prevBtn->setFont(L"Arial", 8);

    // Next Page
    auto nextBtn = std::make_unique<vButton>(m_hInstance, nextId, L"\u25B6", 0, btnY, btnW, btnW, getEventDispatcher());
    m_nextBtn = nextBtn.get();
    m_bottomPanel->addChild(nextId, std::move(nextBtn));
    //m_nextBtn->setFont(L"Arial", 8);

    // Last Page
    auto lastBtn = std::make_unique<vButton>(m_hInstance, lastId, L"\u25B6|", 0, btnY, btnW, btnW, getEventDispatcher());
    m_lastBtn = lastBtn.get();
    m_bottomPanel->addChild(lastId, std::move(lastBtn));
    //m_lastBtn->setFont(L"Arial", 8);

    

    // 5. Înregistrare Handleri (Evenimente)
    // Folosim EventDispatcher-ul centralizat
    getEventDispatcher().registerHandler("click", firstId, [this]() { this->goToFirstPage(); });
    getEventDispatcher().registerHandler("click", prevId, [this]() { this->goToPrevPage(); });
    getEventDispatcher().registerHandler("click", nextId, [this]() { this->goToNextPage(); });
    getEventDispatcher().registerHandler("click", lastId, [this]() { this->goToLastPage(); });

    // 6. Aplicare Layout și starea inițială
    getEventDispatcher().registerHandler(
        "grid_count_complete",
        m_id, // ID-ul tău de control
        [this](const std::string& countStr) {

            long long totalRecords = -1;
            try {
                totalRecords = std::stoll(countStr);
            }
            catch (const std::exception& e) {
                LOG_FATAL(L"Eroare conversie COUNT din thread: " + str_to_wstr(e.what()));
                return;
            }

            // Setează rezultatul și actualizează UI-ul
            if (totalRecords >= 0) {
                this->m_totalRecords = totalRecords;
                // Aici apelezi metoda care actualizează textul de paginare și butoanele:
                //this->updatePaginationUI(totalRecords);
                this->updatePaginationButtonsState();
                //LOG_SUCCESS(L"UI actualizat cu numărul total de rânduri: " + std::to_wstring(totalRecords));
            }
        }
    );


    m_bottomPanel->applyLayout();

    LOG_DEBUG(L"Butonul Next are ID-ul: " + str_to_wstr(m_nextBtn->getId()) +
        L" și Handle-ul: " + std::to_wstring((UINT_PTR)m_nextBtn->getHandle()));

    updatePaginationButtonsState();
}



void vDbFilteredGrid::setupActionControls() {
    if (!m_topPanel) return;

    m_topPanel->clearChildren();

    const int btnW = 28;
    const int spacing = 4;
    const int btnY = (m_topPanel->getHeight() - btnW) / 2;

    // Helper lambda pentru a genera ID-uri prefixate local
    auto makeId = [this](const std::string& subId) { return m_id + "_" + subId; };

    // 1. Creare ID-uri unice
    std::string applyId = makeId("applyFiltersBtn");
    std::string clearId = makeId("clearFiltersBtn");
    std::string addId = makeId("addRecordBtn");
    std::string editId = makeId("editRecordBtn");
    std::string delId = makeId("deleteRecordBtn");
    std::string expId = makeId("exportCsvBtn");

    // 2. Adăugare butoane în Panel
    m_topPanel->addChild(applyId, std::make_unique<vButton>(m_hInstance, applyId, L"\u23F5", spacing, btnY, btnW, btnW, getEventDispatcher()));
    m_topPanel->addChild(clearId, std::make_unique<vButton>(m_hInstance, clearId, L"\u21BB", spacing, btnY, btnW, btnW, getEventDispatcher()));

    if (m_isEditable) {
        m_topPanel->addChild(addId, std::make_unique<vButton>(m_hInstance, addId, L"\u2795", spacing, btnY, btnW, btnW, getEventDispatcher()));
        m_topPanel->addChild(editId, std::make_unique<vButton>(m_hInstance, editId, L"\u270F", spacing, btnY, btnW, btnW, getEventDispatcher()));
        m_topPanel->addChild(delId, std::make_unique<vButton>(m_hInstance, delId, L"\U0001F5D1", spacing, btnY, btnW, btnW, getEventDispatcher()));
    }

    if (m_isExportable) {
        // Folosim \u2312 (simbol de arc/save) sau \U0001F4E5 (download box) sau simplu L"CSV"
        m_topPanel->addChild(expId, std::make_unique<vButton>(m_hInstance, expId, L"\U0001F4E5", spacing, btnY, btnW, btnW, getEventDispatcher()));
    }

    //m_topPanel->addChild("spring2", std::make_unique<vSpacer>("spring2", 0, btnY, 3, btnW, getEventDispatcher()));
    //auto titleLabel = std::make_unique<vLabel>(m_hInstance, "titleLabel", m_title, 0, btnY, labelWidth, btnW, getEventDispatcher());
    auto titleLabel = std::make_unique<vLabel>(m_hInstance, "titleLabel", m_title, spacing, btnY, 400, btnW, getEventDispatcher());
    titleLabel->setTextAlign(TextAlign::LEFT);
    titleLabel->setWidthMode(SizeMode::FILL);
    titleLabel->setMargins(30, 0, 10, 0);
    m_titleLabel = titleLabel.get();
    m_topPanel->addChild("titleLabel", std::move(titleLabel));


    // 3. Înregistrare Handleri pe ID-urile UNICE
    getEventDispatcher().registerHandler("click", applyId, [this]() {
        m_currentPage = 0;
        this->applyFilters();
        });

    getEventDispatcher().registerHandler("click", clearId, [this]() {
        m_currentPage = 0;
        this->clearFilters();
        });

    getEventDispatcher().registerHandler("click", editId, [this]() { this->editRecord(); });
    getEventDispatcher().registerHandler("click", addId, [this]() { this->insertRecord(); });
    getEventDispatcher().registerHandler("click", delId, [this]() { this->deleteRecord(); });


    if (m_isExportable) {
        getEventDispatcher().registerHandler("click", expId, [this]() {
            this->exportToCsv();
            });
    }

    getEventDispatcher().registerHandler("grid_right_click", m_id+"_internal", [this](const std::string& arg) {
        this->handleGridMenu(arg);
        }
    );

    m_topPanel->applyLayout();
}

void vDbFilteredGrid::setupFilterControls() {
    if (!m_filterPanel || !m_grid || !m_isFilterable) return;

    // Curățare vechi
    for (auto* edit : m_filterEdits) m_filterPanel->removeChild(edit->getId());
    m_filterEdits.clear();

    const auto& colNames = m_grid->getColumnNames();
    int currentX_logical = 0;

    for (int i = 0; i < (int)colNames.size(); ++i) {
        int physicalWidth = m_grid->getColumnWidth(i);
        if (physicalWidth <= 0) continue;

        int logicalWidth = MulDiv(physicalWidth, 96, m_currentDpi);
        // PREFIXARE ID: "airports_filter_NumeColoana"
        std::string editId = m_id + "_filter_" + wstr_to_str(colNames[i]);

        auto edit = std::make_unique<vEdit>(m_hInstance, editId, currentX_logical, 0, logicalWidth, 22, getEventDispatcher());
        vEdit* pEdit = edit.get();
        pEdit->setBackgroundColor(RGB(232, 232, 232));
        m_filterPanel->addChild(editId, std::move(edit));
        m_filterEdits.push_back(pEdit);

        // Handler lost_focus pe ID unic
        getEventDispatcher().registerHandler("lost_focus", editId, [this]() {
            this->applyFilters();
            });

        currentX_logical += logicalWidth;
    }
}

// Implementarea `populate`
bool vDbFilteredGrid::populate(const std::wstring& query, const std::string uniqueIdField) {
    vWaitCursor wait;

    m_lastQuery = query;
    m_uniqueIdField = uniqueIdField;

    // Inițializează analizorul cu interogarea originală.
    SqlQueryAnalyzer analyzer(query);

    std::wstring populate_query = query;

    // ---------------------------------------------------------------------
    // NOUA LOGICĂ: Utilizează analizorul pentru a verifica prezența paginării.
    // ---------------------------------------------------------------------

    // NOTĂ: Trebuie să adaugi o metodă 'containsPaginationClause' în SimpleQueryAnalyzer.
    if (!analyzer.containsPaginationClause()) {
        // Interogarea este o interogare completă, adăugăm LIMIT 1 pentru viteză
        populate_query += L" LIMIT 1";
       // ConsoleManager::getInstance().log(L"[vDbFilteredGrid::populate] Adaugat 'LIMIT 1' pentru extragerea rapida a metadatelor.");
    }
    else {
        // Interogarea are deja paginare (LIMIT/FETCH). O folosim așa cum este.
      //  ConsoleManager::getInstance().log(L"[vDbFilteredGrid::populate] Query-ul conține deja LIMIT/FETCH. Se folosește neschimbat.");
    }


    if (m_grid && m_grid->populate(populate_query)) {
        // Creează controalele de filtrare DUPĂ ce grila a fost populată și are coloane
        setupFilterControls();

        // applyFilters va folosi m_lastQuery (originalul) pentru a genera COUNT(*) și interogările paginate.
        applyFilters();
        this->updateSortArrow(m_sortedColumn, m_sortAscending);

        //m_grid->autoFitAllColumns();
        return true;
    }

    ConsoleManager::getInstance().log(L"[ERROR] vDbFilteredGrid::populate: Grila internă (m_grid) este invalidă sau popularea a eșuat.");
    return false;
}

void vDbFilteredGrid::applyFilters() {
    vWaitCursor wait;
    // ------------------------------------------
    // 1. VERIFICĂRI PRELIMINARE
    // ------------------------------------------
    if (m_lastQuery.empty()) return;

    if (!m_db || !m_db->isConnected()) {
        ConsoleManager::getInstance().log(L"[ERROR] applyFilters: Conexiunea la baza de date este invalidă.");
        return;
    }

    // ------------------------------------------
    // 2. CONSTRUIRE FILTRE (Clauza WHERE)
    // ------------------------------------------
    std::wstring filterClause = L"";
    bool firstFilter = true;

    for (const auto& edit : m_filterEdits) {
        std::wstring filterText = edit->getText();
        if (!filterText.empty()) {
            std::string fullId = edit->getId();
            std::string marker = "_filter_";
            size_t pos = fullId.find(marker);
            if (pos != std::string::npos) {
                // Extragem DOAR ce este după "_filter_"
                std::string colNameStr = fullId.substr(pos + marker.length());
                std::wstring columnName = str_to_wstr(colNameStr);

                filterClause += (firstFilter ? L" WHERE " : L" AND ");
                firstFilter = false;

                // Acum columnName va fi "tara", nu "_grid_filter_tara"
                //filterClause += L"CAST(" + columnName + L" AS TEXT) LIKE '%" + filterText + L"%'";
                filterClause += L"CAST(" + columnName + L" AS TEXT) ILIKE '" + filterText + L"%'";
            }
            /*
            // Extragem numele coloanei din ID-ul controlului (ex: "filter_numeColoana")
            std::wstring columnName = str_to_wstr(edit->getId().substr(7));

            filterClause += (firstFilter ? L" WHERE " : L" AND ");
            firstFilter = false;

            // Folosim CAST ca TEXT pentru a permite căutarea LIKE și în coloane numerice (ID, preț, etc.)
            filterClause += L"CAST(" + columnName + L" AS TEXT) LIKE '%" + filterText + L"%'";
            */
        }
    }

    // ------------------------------------------
    // 3. CONSTRUIRE QUERY PENTRU AFIȘARE (Cu Sortare și Paginare)
    // ------------------------------------------

    // Împachetăm query-ul original într-un sub-query pentru a-l izola de filtrele noastre dinamice
    std::wstring wrappedQuery = L"SELECT * FROM (" + m_lastQuery + L") AS src" + filterClause;

    std::wstring finalQuery = wrappedQuery;

    // A. Adăugăm SORTAREA (ORDER BY) - Trebuie să fie înainte de LIMIT
    if (m_sortedColumn != -1) {
        std::wstring dbColName = m_grid->getDbColumnName(m_sortedColumn);
        if (!dbColName.empty()) {
            finalQuery += L" ORDER BY " + dbColName + L" " + (m_sortAscending ? L"ASC" : L"DESC");
        }
    }
    m_lastFilteredQuery = finalQuery;
    // B. Adăugăm PAGINAREA (LIMIT și OFFSET)
    if (m_isPaginable) {
        finalQuery += L" LIMIT " + std::to_wstring(m_recordsPerPage);
        finalQuery += L" OFFSET " + std::to_wstring(m_currentPage * m_recordsPerPage);
    }

    // ------------------------------------------
    // 4. EXECUTARE ȘI ACTUALIZARE GRID
    // ------------------------------------------
    if (m_grid) {
        m_lastFinalQuery = finalQuery;
        m_grid->populate(finalQuery);

        // Actualizăm vizual săgeata de sortare în antetul coloanei Win32
        if (m_sortedColumn != -1) {
            updateSortArrow(m_sortedColumn, m_sortAscending);
        }

        //ConsoleManager::getInstance().log(L"[applyFilters] Executat: " + finalQuery);
    }

    // ------------------------------------------
    // 5. NUMĂRARE ASINCRONĂ (COUNT pentru paginare)
    // ------------------------------------------

    // Interogarea pentru numărare folosește doar filtrele, fără sortare sau paginare
    std::wstring countRowsQuery = L"SELECT COUNT(*) AS nr_rows FROM (" + wrappedQuery + L") AS count_alias;";

    try {
        // Lansăm thread-ul pentru a nu bloca interfața grafică
        m_threads.emplace_back(
            &vDbFilteredGrid::runCountAsync,
            this,
            m_db,
            countRowsQuery,
            &getEventDispatcher(),
            m_id
        );

        // Resetăm contorul până când thread-ul aduce valoarea reală
        m_totalRecords = 0;
    }
    catch (const std::exception& e) {
        LOG_FATAL(L"Eroare thread count: " + str_to_wstr(e.what()));
    }

    // ------------------------------------------
    // 6. ACTUALIZARE UI
    // ------------------------------------------
    updatePaginationButtonsState();
    if (m_grid) m_grid->update();
}

void vDbFilteredGrid::syncFilterScroll() {
    if (!m_grid || !m_filterPanel) return;

    // 1. Obținem poziția orizontală a scroll-ului din Grid
    // LVM_GETORIGIN sau mai simplu prin GetScrollInfo
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;

    if (GetScrollInfo(m_grid->getHandle(), SB_HORZ, &si)) {
        int scrollX = si.nPos;

        // 2. Mutăm panoul de filtre la stânga cu valoarea scroll-ului
        // Menținem Y-ul original și lățimea, dar schimbăm X-ul în negativ
        SetWindowPos(m_filterPanel->getHandle(), NULL,
            -scrollX, m_filterPanel->getY(),
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}


void vDbFilteredGrid::setColumnWidth(const std::wstring& dbColumnName, int width) {
    m_grid->setColumnWidth(dbColumnName, width);
}

unsigned long int vDbFilteredGrid::getRowsCount() const {
    return m_grid->getRowsCount();

}

unsigned long int vDbFilteredGrid::getTotalRrecords() const {
    return m_totalRecords;

}

// În vDbFilteredGrid.cpp
void vDbFilteredGrid::updatePaginationButtonsState() {

    if (!m_isPaginable) return;
    
    // Calculează numărul maxim de pagini
    // Folosim un check pentru a evita împărțirea la zero, deși m_recordsPerPage ar trebui să fie > 0
    if (m_recordsPerPage == 0) return;

    // Calculul corect al numărului total de pagini
    unsigned int maxPages = (m_totalRecords > 0) ? (m_totalRecords + m_recordsPerPage - 1) / m_recordsPerPage : 0;

    // Cazul 1: Nu există înregistrări sau sunt mai puține înregistrări decât o pagină
    bool disableAll = (m_totalRecords <= m_recordsPerPage) || (m_totalRecords == 0);

    if (disableAll) {
        // Dacă nu avem suficiente rânduri pentru a avea mai multe pagini, dezactivăm tot.
        m_firstBtn->setEnabled(false);
        m_prevBtn->setEnabled(false);
        m_nextBtn->setEnabled(false);
        m_lastBtn->setEnabled(false);
    }
    else {
        // Cazul 2: Avem mai multe pagini. Setăm stările direct:

        // --------------------------------------------------------
        // Logica butoanelor de început (First / Previous)
        // Sunt active DOAR dacă NU suntem pe prima pagină (m_currentPage > 0)
        // --------------------------------------------------------
        bool canGoBack = (m_currentPage > 0);
        m_firstBtn->setEnabled(canGoBack);
        m_prevBtn->setEnabled(canGoBack);

        // --------------------------------------------------------
        // Logica butoanelor de sfârșit (Next / Last)
        // Sunt active DOAR dacă NU suntem pe ultima pagină (m_currentPage < maxPages - 1)
        // --------------------------------------------------------
        bool canGoForward = (m_currentPage < maxPages - 1);
        m_nextBtn->setEnabled(canGoForward);
        m_lastBtn->setEnabled(canGoForward);
    }

    // Add logic to update the label text
    if (m_paginationLabel) {
        unsigned int maxPages = (m_totalRecords + m_recordsPerPage - 1) / m_recordsPerPage;
        std::wstring labelText = L"Pagina " + std::to_wstring(m_currentPage + 1) + L" din " + std::to_wstring(maxPages) + L" (" + std::to_wstring(m_totalRecords) + L" înregistrări)";
        m_paginationLabel->setText(labelText);
    }
}

// În vDbFilteredGrid.cpp
void vDbFilteredGrid::goToFirstPage() {
    ConsoleManager::getInstance().log(L"[vDbFilteredGrid::goToFirstPage] Execut goToFirstPage" );
    if (m_currentPage != 0) {
        m_currentPage = 0;
        applyFilters();
    }
}

void vDbFilteredGrid::goToPrevPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        applyFilters();
    }
}

void vDbFilteredGrid::goToNextPage() {
    unsigned int maxPages = (m_totalRecords + m_recordsPerPage - 1) / m_recordsPerPage;
    if (m_currentPage < maxPages - 1) {
        m_currentPage++;
        applyFilters();
    }
}

void vDbFilteredGrid::goToLastPage() {
    unsigned int maxPages = (m_totalRecords + m_recordsPerPage - 1) / m_recordsPerPage;
    if (m_currentPage < maxPages - 1) {
        m_currentPage = maxPages - 1;
        applyFilters();
    }
}

void vDbFilteredGrid::setRecordsPerPage(int recordsPerPage) {
    if (recordsPerPage <= 0) {
      //  ConsoleManager::getInstance().log(L"[WARNING] vDbFilteredGrid::setRecordsPerPage: Valoare invalidă (" + std::to_wstring(recordsPerPage) + L"). Numărul de înregistrări per pagină trebuie să fie mai mare ca 0.");
        return;
    }

    if (recordsPerPage != m_recordsPerPage) {
        m_recordsPerPage = recordsPerPage;
        m_currentPage = 0; // Resetăm pagina la 0, deoarece dimensiunea paginii s-a schimbat

        // Se aplică filtrele și se repopulează grila cu noile setări de paginare
        applyFilters();
    }
}

void vDbFilteredGrid::clearFilters() {
 //   ConsoleManager::getInstance().log(L"[vDbFilteredGrid::clearFilters] Se sterg filtrele.");

    // Golește textul fiecărui câmp de editare
    for (const auto& edit : m_filterEdits) {
        edit->setText(L"");
    }

    // Resetează pagina curentă la prima pagină
    m_currentPage = 0;
    this->resetSort();
    // Apelează applyFilters pentru a repopula grila cu filtrele goale
    applyFilters();
}


// Aceasta este funcția de lucru, rulată pe thread-ul separat
void vDbFilteredGrid::runCountAsync(dbConnection* db,
    const std::wstring& countQuery,
    EventDispatcher* dispatcher,
    const std::string& controlId)
{

    //LOG_DEBUG(L"RULELZ Count:" + countQuery);
    if (m_shuttingDown)
        return;

    //LOG_INFO(L"[CountThread] Thread de numărare pornit.");

    long long count = -1;

    if (!m_shuttingDown)
        count = db->execCountQuery(countQuery);

    if (m_shuttingDown)
        return;

    if (count >= 0) {
        // LOG_SUCCESS(L"[CountThread] Numărare finalizată. Total rânduri: " + std::to_wstring(count));
    }
    else {
        LOG_ERROR(L"[CountThread] Numărare eșuată.");
    }

    if (!m_shuttingDown)
        dispatcher->dispatch("grid_count_complete", controlId, std::to_string(count));

   // LOG_SUCCESS(L"[CountThread] Thread de numărare terminat.");
}


vDbFilteredGrid::~vDbFilteredGrid() {
    m_shuttingDown = true;

    for (auto& t : m_threads) {
        if (t.joinable())
            t.join();
    }
}

void vDbFilteredGrid::updateSortArrow(int columnIndex, bool ascending) {
    if (!m_grid) return;
    m_grid->updateSortArrow(columnIndex, m_sortAscending);
}

void vDbFilteredGrid::handleGridMenu(const std::string& argument) {
    // 1. Parsare argumente (Index rând, X, Y)
    std::stringstream ss(argument);
    std::string item;
    std::vector<int> vals;
    try {
        while (std::getline(ss, item, ';')) {
            vals.push_back(std::stoi(item));
        }
    }
    catch (...) { return; }
    if (vals.size() < 3) return;


    int row = m_grid->getSelectedRow();
    //vMessageDialog::Info(L"Row: " + to_wstring(row) + L" - " +str_to_wstr(m_uniqueIdField));
    if (row < 0) return;
    std::wstring selectedId = m_grid->getCellValueByFieldName(row, m_uniqueIdField);


    int rowIndex = vals[0];
    int x = vals[1];
    int y = vals[2];

    // --- MODIFICARE AICI: ID unic pentru meniu ---
    std::string menuId = m_id + "_contextMenu";
    vPopupMenu menu(menuId, m_dispatcher);
    menu.create(nullptr);

    // 2. Configurare iteme
    menu.addItem("reload", L"Reîncărcare");
    if (m_isEditable) {
        menu.addSeparator("sep0");
        menu.addItem("add", L"Adaugă înregistrare");
        //menu.addItem("edit", L"Editează înregistrarea: " + std::to_wstring(rowIndex));
        //menu.addItem("delete", L"Șterge înregistrarea: " + std::to_wstring(rowIndex));

        menu.addItem("edit", L"Editează înregistrarea: " + to_wstring(row) + L" (" + selectedId + L")");
        menu.addItem("delete", L"Șterge înregistrarea: " + to_wstring(row) + L" (" + selectedId + L")");
    }

    if (m_isExportable) {
        menu.addSeparator("sep_exp");
        menu.addItem("export_grid_csv", L"Exportă datele filtrate (CSV)");
    }

    // 2.1 EXTENSIA DIN EXTERIOR
    if (m_onContextMenuExtend) {
        menu.addSeparator("sep_custom");
        // Permitem codului de afară să adauge ce vrea
        m_onContextMenuExtend(menu, row, selectedId);
    }

    // 3. Afișare (TrackPopupMenu blochează execuția până la selecție)
    SetForegroundWindow(m_grid->getHandle());
    int sel = menu.display(m_grid->getHandle(), x, y);

    // 4. Procesare selecție
    if (sel > 0) {
        // Obținem ID-ul string (ex: "edit") bazat pe ID-ul numeric Win32
        std::string action = menu.getItemIdByWin32Id(sel);

        // Debug Log
        ConsoleManager::getInstance().log(L"[Menu Action] Executare: " +
            str_to_wstr(action) + L" pe ID: " + str_to_wstr(m_id));

        // Acțiunile acum vor apela corect instanța ferestrei curente (this)
        if (action == "edit") {
            this->editRecord();
        }
        else if (action == "add") {
            this->insertRecord();
        }
        else if (action == "delete") {
            this->deleteRecord();
        }
        else if (action == "reload") {
            this->applyFilters();
        }
        else if (action == "export_grid_csv") {
            this->exportToCsv();
        }
        else {
            m_dispatcher.dispatch("custom_menu_action", action, wstr_to_str(selectedId));
        }
    }
}

void vDbFilteredGrid::updateStatusWithSelection(int rowIndex) {
    if (!m_statusBar || m_uniqueIdField.empty()) return;

    // 1. Obținem datele rândului din Grid
    // Presupun că vDbGrid are o metodă să returneze datele sub formă de map sau direct valoarea pe coloană
    m_uniqueIdValue = m_grid->getCellValueByFieldName(rowIndex, m_uniqueIdField);

    // 2. Formatăm mesajul
    std::wstring statusText = L"Înregistrare selectată (" + str_to_wstr(m_uniqueIdField) + L": " + m_uniqueIdValue + L")";
    ;
    // 3. Trimitem către Status Bar (presupunând că vStatusBar are o metodă setText)
    m_statusBar->setText(statusText);
}

bool vDbFilteredGrid::editRecord() {
    // 1. Identificăm ID - ul selectat în grid
    int row = m_grid->getSelectedRow();
    if (row < 0) return false;

    std::wstring selectedId = m_grid->getCellValueByFieldName(row, m_uniqueIdField);

    // 2. Trimitem "comanda" către exterior prin callback
    if (m_onDbEdit) {

        m_onDbEdit(DbDialogMode::Update, selectedId);
        return true;
    }
    return false;
}

bool vDbFilteredGrid::insertRecord() {
    if (m_onDbAdd) {

        m_onDbAdd(DbDialogMode::Insert);
        return true;
    }
    return false;
    
}

bool vDbFilteredGrid::deleteRecord() {
    // 1. Identificăm ID - ul selectat în grid
    int row = m_grid->getSelectedRow();
    if (row < 0) return false;

    std::wstring selectedId = m_grid->getCellValueByFieldName(row, m_uniqueIdField);

    // 2. Trimitem "comanda" către exterior prin callback
    if (m_onDbDel) {

        m_onDbDel(DbDialogMode::Delete, selectedId);
        return true;
    }
    return false;
    
   
}


bool vDbFilteredGrid::viewRecord() {
    // 1. Identificăm ID - ul selectat în grid
    int row = m_grid->getSelectedRow();
    if (row < 0) return false;

    std::wstring selectedId = m_grid->getCellValueByFieldName(row, m_uniqueIdField);

    // 2. Trimitem "comanda" către exterior prin callback
    if (m_onDbView) {

        m_onDbView(DbDialogMode::View, selectedId);
        return true;
    }
    return false;


}


/*
void vDbFilteredGrid::setEditWindow(std::unique_ptr<vDbEditDialog> editDialog) {
    if (!editDialog) return;
    // Salvăm adresa pentru a putea apela ShowWindow mai târziu
    m_EditDialog = editDialog.get();
     // Transferăm proprietatea către vApp pentru managementul duratei de viață
    
    vApp::getAppInstance()->addWindow(editDialog->getId(), std::move(editDialog));
    
}
*/


LRESULT vDbFilteredGrid::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        /*
    case 1125: // Mesajul tău custom de scroll (WM_USER + 101)
    {
        int scrollX = (int)wParam;
        if (m_filterPanel && !m_filterEdits.empty() && m_grid)
        {
            int currentX = 0;
            // Presupunem că grid-ul are același gap ca layout-ul filtrelor
            // Dacă ai gap, preia-l: int gap = m_grid->getGap(); 

            HDWP hdwp = BeginDeferWindowPos((int)m_filterEdits.size());

            for (size_t i = 0; i < m_filterEdits.size(); ++i)
            {
                int colW = m_grid->getColumnWidthByIndex((int)i);
                int targetX = currentX - scrollX;
                int panelH = m_filterPanel->getHeight();

                hdwp = DeferWindowPos(hdwp,
                    m_filterEdits[i]->getHandle(),
                    NULL,
                    targetX, 0, colW, panelH,
                    SWP_NOZORDER | SWP_NOACTIVATE);

                currentX += colW; // Dacă ai gap, adaugă aici: currentX += (colW + gap);
            }
            EndDeferWindowPos(hdwp);
        }
        return 0;
    }*/
    case 1125: // WM_USER + 101 (Scroll)
    case 1126: // WM_USER + 102 (Resize Coloană)
    {
        // Obținem scroll-ul curent (esențial chiar și la resize)
        int scrollX = GetScrollPos(m_grid->getHandle(), SB_HORZ);

        if (m_filterPanel && !m_filterEdits.empty() && m_grid)
        {
            int currentX = 0;
            HDWP hdwp = BeginDeferWindowPos((int)m_filterEdits.size());

            for (size_t i = 0; i < m_filterEdits.size(); ++i)
            {
                // Luăm lățimea nouă a coloanei (proaspăt redimensionată)
                int colW = m_grid->getColumnWidthByIndex((int)i);
                int targetX = currentX - scrollX;
                int panelH = m_filterPanel->getHeight();

                // Verificăm dacă e vizibil înainte de a apela ShowWindow
                bool isVisible = IsWindowVisible(m_filterEdits[i]->getHandle());

                if (colW < 5) {
                    if (isVisible) ShowWindow(m_filterEdits[i]->getHandle(), SW_HIDE);
                }
                else {
                    if (!isVisible) ShowWindow(m_filterEdits[i]->getHandle(), SW_SHOW);
                }

                hdwp = DeferWindowPos(hdwp,
                    m_filterEdits[i]->getHandle(),
                    NULL,
                    targetX, 0, colW, panelH,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

                currentX += colW;
            }
            EndDeferWindowPos(hdwp);
            InvalidateRect(m_filterPanel->getHandle(), NULL, TRUE);
            UpdateWindow(m_filterPanel->getHandle());
        }
        return 0;
    }

    case WM_COMMAND: {
        int controlId = LOWORD(wParam);
        int notificationCode = HIWORD(wParam);


        // 2. Tratăm notificările specifice de la Edits (Filtre)
        // Dacă ai subclassing-ul activ, poate trimite codul 0 sau un cod custom.
        // Dacă vrei să prinzi momentul când utilizatorul pleacă din căsuță:
        if (notificationCode == EN_KILLFOCUS) {
            this->applyFilters();
            return 0;
        }

        // 3. Logica existentă pentru butoane (BN_CLICKED)
        if (notificationCode == BN_CLICKED) {
            // Căutăm controlul în lista noastră (vPanel are acces la m_controlsByWin32Id)
            vControl* child = getChildByWin32Id(controlId);
            if (child) {
                child->onClick();
                return 0;
            }
        }
        break;
    }

    case WM_NOTIFY: {
        LPNMHDR lpnmhdr = reinterpret_cast<LPNMHDR>(lParam);

        // Verificăm dacă notificarea vine de la grid și este un click pe coloană
        if (m_grid && lpnmhdr->hwndFrom == m_grid->getHandle()) {
            if (lpnmhdr->code == LVN_COLUMNCLICK) {
                LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
                int columnIndex = pnmv->iSubItem;
                // Logică de toggle pentru direcția sortării
                if (m_sortedColumn == columnIndex) {
                    m_sortAscending = !m_sortAscending;
                }
                else {
                    m_sortedColumn = columnIndex;
                    m_sortAscending = true;
                }

                this->updateSortArrow(m_sortedColumn, m_sortAscending);
                // Aplicăm sortarea refăcând query-ul la baza de date
                m_currentPage = 0;
                this->applyFilters();
                return 0;
            }
            if (lpnmhdr->code == LVN_ITEMCHANGED) {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;

                // Verificăm dacă un rând a fost selectat (starea s-a schimbat în 'selected')
                if ((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED)) {
                    int rowIndex = pnmv->iItem;
                    updateStatusWithSelection(rowIndex);
                }
            }
        }

        break;
    }



    default:
        break;
    }

    // Foarte important: lasă clasa de bază să proceseze restul
    return vPanel::handleMessage(hwnd, msg, wParam, lParam);
}


void vDbFilteredGrid::setEditable(bool editable){
    m_isEditable = editable; 
    if (m_topPanel) {
        setupActionControls();
    } 
}


void vDbFilteredGrid::setExportable(bool exportable) {
    m_isExportable = exportable;
    
    if (m_topPanel) {
        setupActionControls();
    }
}

void vDbFilteredGrid::setPaginable(bool paginable) {
    m_isPaginable = paginable;

    if (m_bottomPanel) {
        setupPagingControls();
    }
}


void vDbFilteredGrid::setFilterable(bool filterable) {
    m_isFilterable = filterable;

    if (m_filterPanel) {
        setupFilterControls();
    }
}

void vDbFilteredGrid::resetSort() {
    // 1. Resetăm starea internă
    m_sortedColumn = -1;
    m_sortAscending = true;

    // 2. Eliminăm săgeata de sortare din Header-ul Win32
    // Trimitem -1 pentru a indica faptul că nicio coloană nu mai are prioritate de sortare
    updateSortArrow(-1, true);

    // 3. Re-aplicăm filtrele (care va reconstrui query-ul fără ORDER BY)
    applyFilters();

    //LOG_INFO(L"[vDbFilteredGrid] Sortarea a fost resetată.");
}
/*
void vDbFilteredGrid::showCellContent(int rowIndex, int colIndex, const std::wstring& content) {
    // Acum compilatorul va găsi metoda getColumnName în m_grid (vDbGrid)
    std::wstring colName = m_grid->getColumnName(colIndex);

    std::wstring message = L"Detalii înregistrare:\n"
        L"----------------------\n"
        L"Tabel: " + str_to_wstr(m_id) + L"\n" +
        L"Rând: " + std::to_wstring(rowIndex + 1) + L"\n" +
        L"Câmp: " + colName + L"\n" +
        L"Valoare: " + content;

    // Folosim MessageBoxW pentru suport Unicode complet
    MessageBoxW(m_handle, message.c_str(), L"Vizualizare Conținut Celulă", MB_OK | MB_ICONINFORMATION);

    LOG_DEBUG(L"[vDbFilteredGrid] showCellContent afișat pentru: " + colName);
}
*/

void vDbFilteredGrid::showCellContent(int rowIndex, int colIndex, const std::wstring& content) {
    std::wstring colName = m_grid->getColumnName(colIndex);
    std::wstring title = L"Conținut: " + colName;

    // 1. Cream o fereastră de tip ToolWindow (mai mică, fără butoane de minimizat)
    auto detailWin = std::make_unique<vWindow>(
        m_hInstance,
        "cell_detail_win",
        WindowType::ToolWindow,
        false,
        getEventDispatcher()
        );

    // Folosim un AnchorLayout pentru a întinde edit-ul automat
    detailWin->setLayoutStrategy(std::make_unique<AnchorLayout>());

    // 2. Cream fereastra fizică (dimensiuni rezonabile)
    //detailWin->create(L"VAppDetailClass", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 250, m_handle);
    detailWin->create(
        L"VAppDetailClass",
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 300,
        m_handle // m_handle este grid-ul părinte, ajută la focalizare
    );
    detailWin->centerWindow();

    // 3. Adăugăm un control vEdit multi-line
    // Folosim ES_READONLY pentru a permite copy, dar nu și modificarea accidentală
    auto edit = std::make_unique<vEdit>(
        m_hInstance,
        "cell_detail_edit",
        0, 0, 100, 100,
        getEventDispatcher(),
        EditType::MULTI_LINE
        );
    
    edit->setHeightMode(SizeMode::FILL);
    edit->setWidthMode(SizeMode::FILL);
    edit->setMargins(2, 2, 2, 2); // Un pic de padding pentru estetică

    vEdit* pEdit = edit.get();
    detailWin->addChild("detail_edit", std::move(edit));

    // 4. Aplicăm layout-ul și setăm textul
    detailWin->applyLayout();
    pEdit->setText(content);
    pEdit->setReadOnly(true);
    // 5. Opțional: Putem face fereastra să se închidă la tasta ESC sau să fie modală
    // Pentru simplitate, o afișăm normal (show), dar dacă vrei să blochezi restul UI-ului folosește showModal()
    detailWin->show();

    // Notă: Trebuie să avem grijă de memoria lui detailWin. 
    // Dacă îl facem local (std::unique_ptr), se va distruge la ieșirea din funcție.
    // O variantă este să îl adaugi ca membru în vDbFilteredGrid sau să îl "uiți" (leak controlat sau self-destroy)
    // Cea mai curată metodă: îl adăugăm ca fereastră gestionată în ANCTerminal sau într-un manager de ferestre.

    // Pentru acest test, îl vom "muta" într-un container de ferestre active dacă ai unul, 
    // sau îl lăsăm ca variabilă membru m_activeDetailWin în vDbFilteredGrid.
    m_activeDetailWin = std::move(detailWin);
}



void vDbFilteredGrid::onCellDblClick(int rowIndex, int colIndex, const std::wstring& content) {
    if (m_customCellDblClick) {
        // Dacă am setat o funcție custom, o executăm pe aceea
        m_customCellDblClick(rowIndex, colIndex, content);
    }
    else {
        // Altfel, executăm comportamentul default (MessageBox-ul)
        this->showCellContent(rowIndex, colIndex, content);
    }
}



bool vDbFilteredGrid::exportToCsv() {
    if (!m_isExportable) return false;

    // 1. Verificăm dacă avem ce exporta
    std::wstring query = getLastFilteredQuery();
    if (query.empty()) {
        LOG_ERROR(L"Export eșuat: Nu există date (query vid).");
        return false;
    }

    // 2. Deschidem dialogul de salvare
    vSaveFileDialog saveDlg(L"Exportă datele în format CSV");
    saveDlg.setFilter(L"CSV Files (*.csv)\0*.csv\0Text Files (*.txt)\0*.txt\0");
    saveDlg.setDefaultExtension(L"csv");

    // Dacă utilizatorul apasă "Cancel", returnăm false fără eroare
    if (!saveDlg.show(this->getHandle())) {
        LOG_INFO(L"Export anulat de utilizator.");
        return false;
    }

    std::wstring filePath = saveDlg.getFilePath();
    std::string tag = "grid_export_task";

    // 3. Executăm query-ul pe baza de date
    if (!m_db->execQuery(query, tag)) {
        LOG_ERROR(L"Export eșuat: Eroare SQL la execuție.");
        return false;
    }

    // 4. Pregătim fișierul
    //std::ofstream outFile(filePath, std::ios::binary);
    std::ofstream outFile{ std::filesystem::path(filePath), std::ios::binary };
    if (!outFile.is_open()) {
        LOG_ERROR(L"Export eșuat: Nu s-a putut crea fișierul pe disc.");
        return false;
    }

    try {
        // Scriem UTF-8 BOM pentru ca Excel să recunoască diacriticele românești
        const unsigned char BOM[] = { 0xEF, 0xBB, 0xBF };
        outFile.write(reinterpret_cast<const char*>(BOM), 3);

        // 5. Scriem Header-ul (Numele coloanelor din DB)
        auto columns = m_db->getColumnNames(tag);
        std::wstring headerLine;
        for (size_t i = 0; i < columns.size(); ++i) {
            headerLine += L"\"" + columns[i] + L"\"";
            if (i < columns.size() - 1) headerLine += L",";
        }
        headerLine += L"\n";
        //std::string utf8Header = WStrToUTF8(headerLine);
        std::string utf8Header = wstring_to_utf8(headerLine);
        outFile.write(utf8Header.c_str(), utf8Header.size());

        // 6. Scriem Datele
        int rowCount = 0;
        while (m_db->fetchNextRow(tag)) {
            std::vector<std::wstring> rowData = m_db->fetchRow(tag);
            std::wstring dataLine;

            for (size_t i = 0; i < rowData.size(); ++i) {
                std::wstring val = rowData[i];

                // Sanitizare: eliminăm line-breaks care strică structura CSV
                size_t n = val.find_first_of(L"\r\n");
                while (n != std::wstring::npos) {
                    val[n] = L' ';
                    n = val.find_first_of(L"\r\n", n + 1);
                }

                // Escapare ghilimele duble (standard RFC 4180)
                size_t pos = val.find(L"\"");
                while (pos != std::wstring::npos) {
                    val.replace(pos, 1, L"\"\"");
                    pos = val.find(L"\"", pos + 2);
                }

                dataLine += L"\"" + val + L"\"";
                if (i < rowData.size() - 1) dataLine += L",";
            }
            dataLine += L"\n";
            //std::string utf8Row = WStrToUTF8(dataLine);
            std::string utf8Row = wstring_to_utf8(dataLine);
            outFile.write(utf8Row.c_str(), utf8Row.size());
            rowCount++;
        }

        outFile.flush();
        outFile.close();

        LOG_SUCCESS(L"Export finalizat cu succes: " + std::to_wstring(rowCount) + L" rânduri.");
        return true;
    }
    catch (...) {
        LOG_ERROR(L"Export eșuat: Eroare neprevăzută la scrierea fișierului.");
        outFile.close();
        return false;
    }
}