#include "vDbGrid.hpp"
#include "..\stringUtils.hpp"
#include "ConsoleManager.hpp"
#include "IconManager.hpp"
#include <vector>
#include <sstream>
#include <algorithm>


// ... constructor, clear, etc. rămân neschimbate ...
// Constructor
vDbGrid::vDbGrid(
    HINSTANCE hInstance,
    const std::string& id,
    int x, int y, int width, int height,
    EventDispatcher& dispatcher,
    dbConnection* db
)
    : vGrid(hInstance, id, x, y, width, height, dispatcher),
    m_db(db)
     {
    if (!m_db || !m_db->isConnected()) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbGrid::Constructor: Conexiunea la baza de date este invalidă.");
        // Poți lansa o excepție aici sau poți gestiona eroarea mai târziu.
    }
}

// Metodă pentru a curăța grid-ul

void vDbGrid::clear() {
    if (m_handle) {
        // Resetăm coloana selectată vizual
        SendMessage(m_handle, LVM_SETSELECTEDCOLUMN, (WPARAM)-1, 0);

        ListView_DeleteAllItems(m_handle);
        while (ListView_DeleteColumn(m_handle, 0));
        m_columnIndex = 0;
        //ConsoleManager::getInstance().log(L"[vDbGrid::clear] Grid-ul a fost golit.");
    }
}
/*
void vDbGrid::clear() {
    if (m_handle) {
        // Șterge toate coloanele
        while (ListView_DeleteColumn(m_handle, 0));

        // Șterge toate rândurile
        ListView_DeleteAllItems(m_handle);

        // Resetează contorul de coloane
        m_columnIndex = 0;

        ConsoleManager::getInstance().log(L"[vDbGrid::clear] Grid-ul a fost golit.");
    }
}
*/
/*
bool vDbGrid::populate(const std::wstring& query ){
//bool vDbGrid::populate(const std::wstring& query) {

    // 1. Curăță datele vechi
   // clear();

    if (!m_db || !m_db->isConnected()) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbGrid::populate: Conexiunea la baza de date nu este activă sau pointerul este NULL.");
        return false;
    }

    //m_lastQuery = query;
    //std::wstring fullQuery = m_lastQuery;
    std::wstring fullQuery = query;
    std::string stm_name = m_id;

    
   
    ConsoleManager::getInstance().log(L"[vDbGrid::populate] Execut query: " + fullQuery);

    if (!m_db->execQuery(fullQuery, stm_name)) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbGrid::populate: Execuția query-ului a eșuat. Eroare: " + m_db->getError());
        return false;
    }

    clear();
    m_columnNames.clear();

    // Setează numele coloanelor din baza de date pentru sortare
    m_columnNames = m_db->getColumnNames(stm_name);

    m_rowsCount = m_db->getRowCount(stm_name);

    // Adaugă coloanele în grid folosind etichetele personalizate
    for (const auto& dbColumnName : m_columnNames) {
        std::wstring displayLabel = dbColumnName; // Numele implicit

        int columnWidth = 150; // Lățimea implicită
        // Verifică dacă există o etichetă personalizată
        auto it = m_columnLabels.find(dbColumnName);
        if (it != m_columnLabels.end()) {
            displayLabel = it->second;
        }

        // Verifică dacă există o lățime personalizată
        auto widthIt = m_columnWidths.find(dbColumnName);
        if (widthIt != m_columnWidths.end()) {
            columnWidth = widthIt->second;
        }

        addColumn(displayLabel, columnWidth);
    }

    // Adaugă rândurile în grid
    while (m_db->fetchNextRow(stm_name)) {
        std::vector<std::wstring> row = m_db->fetchRow(stm_name);
        addRow(row);
    }

   
    ConsoleManager::getInstance().log(L"[vDbGrid::populate] Grid-ul a fost populat cu succes din query.");

   // this->resize();

    return true;
}
*/

bool vDbGrid::populate(const std::wstring& query) {

    if (!m_db || !m_db->isConnected()) return false;

    m_query = query;
    

    std::string stm_name = m_id;
    if (!m_db->execQuery(query, stm_name)) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbGrid::populate: Query failed.");
        return false;
    }

    // Preluăm noile nume de coloane
    std::vector<std::wstring> newColumnNames = m_db->getColumnNames(stm_name);

    // VERIFICARE: Dacă structura coloanelor este identică, NU le mai ștergem
    bool columnsChanged = (newColumnNames != m_columnNames);

    if (columnsChanged) {
        clear(); // Șterge tot (rânduri și coloane)
        m_columnNames = newColumnNames;

        for (const auto& dbColName : m_columnNames) {
            std::wstring label = (m_columnLabels.count(dbColName)) ? m_columnLabels[dbColName] : dbColName;
            int width = (m_columnWidths.count(dbColName)) ? m_columnWidths[dbColName] : 150;
           // addColumn(label, MulDiv(width, m_currentDpi, 96));

            //LOG_DEBUG(L"dbgrid Adaug coloana " + label + L" cu dim de baza sper:" + to_wstring<int>(width));

            addColumn(label, width);
        }
    }
    else {
        // Dacă coloanele sunt la fel, ștergem doar rândurile vechi
        //LOG_ERROR(L"STERG LINII VECHI!");
        ListView_DeleteAllItems(m_handle);
      
    }

    m_rowsCount = m_db->getRowCount(stm_name);

    // Adăugăm rândurile noi
    while (m_db->fetchNextRow(stm_name)) {
        addRow(m_db->fetchRow(stm_name));
    }

    return true;
}




/*
LRESULT vDbGrid::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    //ConsoleManager::getInstance().log(L"[vDbGrid::handleMessage] Sunt in handleMessage.");

    /*

    switch (msg) {
    case WM_NOTIFY: {
        LPNMHDR lpnmhdr = reinterpret_cast<LPNMHDR>(lParam);
        HWND hHeader = ListView_GetHeader(m_handle);

        
        // Log granular pentru a depana sursa și tipul notificării
        //ConsoleManager::getInstance().log(L"  - Cod notificare: " + std::to_wstring(lpnmhdr->code));
        //ConsoleManager::getInstance().log(L"  - Sursa notificării HWND: " + std::to_wstring(reinterpret_cast<uintptr_t>(lpnmhdr->hwndFrom)));
        //ConsoleManager::getInstance().log(L"  - HWND grid: " + std::to_wstring(reinterpret_cast<uintptr_t>(m_handle)));
        //ConsoleManager::getInstance().log(L"  - HWND header: " + std::to_wstring(reinterpret_cast<uintptr_t>(hHeader)));
        
        // Verifică dacă notificarea provine de la grilă sau de la antet
        if (lpnmhdr->hwndFrom == m_handle || lpnmhdr->hwndFrom == hHeader) {
            switch (lpnmhdr->code) {
            case LVN_COLUMNCLICK: {
                LPNMLISTVIEW lpnmlv = reinterpret_cast<LPNMLISTVIEW>(lParam);
                onColumnClick(lpnmlv->iSubItem);
                return TRUE;
            }

            case LVN_ITEMCHANGED: {
                LPNMLISTVIEW pNMLV = (LPNMLISTVIEW)lParam;
                if ((pNMLV->uOldState & LVIS_SELECTED) == 0 && (pNMLV->uNewState & LVIS_SELECTED)) {
                    ConsoleManager::getInstance().log(L"[vDbGrid::handleMessage] Primit LVN_ITEMCHANGED. Un rând a fost selectat.");
                    onClick();
                }
                return 0;
            }

            case NM_DBLCLK: {
                ConsoleManager::getInstance().log(L"[vDbGrid::handleMessage] Primit NM_DBLCLK.");
                onDoubleClick();
                return TRUE;
            }

            case NM_KILLFOCUS: {
                ConsoleManager::getInstance().log(L"[vDbGrid::handleMessage] Primit NM_KILLFOCUS.");
                onKillFocus();
                return 0;
            }
           
            default: {
                //ConsoleManager::getInstance().log(L"[vDbGrid::handleMessage] Trimit catre vGrid pentru WM_NOTIFY");
                return vGrid::handleMessage(hwnd, msg, wParam, lParam);
            }
            }
        }
        break; // Ieșire din case WM_NOTIFY
    }
    }
    
    // Apelează handler-ul clasei de bază pentru a procesa restul mesajelor
    return vGrid::handleMessage(hwnd, msg, wParam, lParam);
}
*/
// Logica de sortare: Acum doar notifică părintele
/*
void vDbGrid::onColumnClick(int columnIndex) {
    ConsoleManager::getInstance().log(L"[vDbGrid::onColumnClick] Coloana " + std::to_wstring(columnIndex) + L" a fost apasata. Trimit eveniment.");

    // NU MAI SCHIMBĂM STAREA DE SORTARE AICI. 
    // Doar trimitem evenimentul.

    // ATENȚIE: Trebuie să trimiți indicele coloanei în eveniment, dacă nu folosești m_sortedColumn
    // Soluție: Folosește un eveniment specific cu columnIndex
    getEventDispatcher().dispatch("grid_column_click", m_id, std::to_string(columnIndex));

    // Vom muta logica de sortare din vechiul onColumnClick în handlerul de eveniment din vDbFilteredGrid.
}
*/

/*
void vDbGrid::updateSortArrow(int columnIndex, bool ascending) {
    if (!m_handle || columnIndex < 0) {
        return;
    }

    HWND hHeader = ListView_GetHeader(m_handle);
    if (!hHeader) {
        return;
    }

    // Obține textul original al coloanei
    wchar_t buffer[256];
    HDITEM hdItem;
    hdItem.mask = HDI_TEXT;
    hdItem.pszText = buffer;
    hdItem.cchTextMax = sizeof(buffer) / sizeof(buffer[0]);

    if (!SendMessage(hHeader, HDM_GETITEM, columnIndex, (LPARAM)&hdItem)) {
        return;
    }

    std::wstring originalText = buffer;

    // Elimină eventualele săgeți deja prezente
    if (!originalText.empty() &&
        (originalText[0] == L'\u2191' || originalText[0] == L'\u2193')) {
        originalText = originalText.substr(1);
    }

    // Adaugă săgeata corespunzătoare
    //std::wstring newText = (ascending ? L"\u2191" : L"\u2193") + originalText;
    //std::wstring newText = (ascending ? L"\u2191 " : L"\u2193 ") + originalText;
    std::wstring newText = (ascending ? L"\u2B06 " : L"\u2B07 ") + originalText;


    // Setează noul text
    hdItem.mask = HDI_TEXT;
    hdItem.pszText = const_cast<wchar_t*>(newText.c_str());

    SendMessage(hHeader, HDM_SETITEM, columnIndex, (LPARAM)&hdItem);
}
*/


/*
void vDbGrid::updateSortIconArrow(int columnIndex, bool ascending) {
    if (!m_handle || columnIndex < 0) {
        return;
    }

    HWND hHeader = ListView_GetHeader(m_handle);
    if (!hHeader) {
        return;
    }

    // Creează imagelist-ul dacă nu există
    if (!m_headerImageList) {
        m_headerImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);

        HICON hIconUp = IconManager::getInstance().getIcon(L"D:\\Programming\\vreport\\libvreport\\imgs\\up_arrow_48.ico", 16, 16);
        HICON hIconDown = IconManager::getInstance().getIcon(L"D:\\Programming\\vreport\\libvreport\\imgs\\down_arrow_48.ico", 16, 16);

        if (hIconUp) m_sortUpIndex = ImageList_AddIcon(m_headerImageList, hIconUp);
        if (hIconDown) m_sortDownIndex = ImageList_AddIcon(m_headerImageList, hIconDown);

        Header_SetImageList(hHeader, m_headerImageList);
    }

    // Curăță pictogramele de pe toate coloanele
    HDITEM hdItemClear;
    hdItemClear.mask = HDI_FORMAT | HDI_IMAGE;
    for (int i = 0; i < m_columnIndex; ++i) {
        if (SendMessage(hHeader, HDM_GETITEM, i, (LPARAM)&hdItemClear)) {
            hdItemClear.fmt &= ~(HDF_IMAGE | HDF_SORTUP | HDF_SORTDOWN);
            hdItemClear.iImage = -1;
            SendMessage(hHeader, HDM_SETITEM, i, (LPARAM)&hdItemClear);
        }
    }

    // Setează pictograma pe coloana sortată
    HDITEM hdItem;
    hdItem.mask = HDI_FORMAT | HDI_IMAGE;
    if (!SendMessage(hHeader, HDM_GETITEM, columnIndex, (LPARAM)&hdItem)) {
        return;
    }

    hdItem.fmt |= HDF_IMAGE;
    hdItem.iImage = ascending ? m_sortUpIndex : m_sortDownIndex;
    hdItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    hdItem.fmt |= ascending ? HDF_SORTUP : HDF_SORTDOWN;

    SendMessage(hHeader, HDM_SETITEM, columnIndex, (LPARAM)&hdItem);
}
*/


/*
// Adaugă implementarea pentru destructor
vDbGrid::~vDbGrid() {
    if (m_headerImageList) {
        ImageList_Destroy(m_headerImageList);
        m_headerImageList = nullptr;
    }


    // Nu este necesară o logică specială aici, vControl::`~vControl` va distruge HWND-ul
}
*/


const std::vector<std::wstring>& vDbGrid::getColumnNames() const {
    return m_columnNames;
}

const std::map<std::wstring, int>& vDbGrid::getColumnWidths() const {
    return m_columnWidths;
}

/*
void vDbGrid::onDoubleClick() {
    ConsoleManager::getInstance().log(L"[vDbGrid::onDoubleClick] Dublu-click gestionat.");

    int selectedIndex = ListView_GetNextItem(m_handle, -1, LVNI_SELECTED);

    if (selectedIndex != -1) {
        // Obține conținutul primei coloane
        WCHAR buffer[256];
        //ListView_GetItemText(m_handle, selectedIndex, 0, buffer, sizeof(buffer) / sizeof(WCHAR));
        ListView_GetItemText(m_handle, selectedIndex, 0, buffer, sizeof(buffer) / sizeof(WCHAR));

        // Stochează textul într-un membru al clasei
        m_selectedCellText = buffer;

        // Declanșează evenimentul pentru dublu-click
        getEventDispatcher().dispatch("grid_double_click");

        ConsoleManager::getInstance().log(L"[vDbGrid::onDoubleClick] Valoare stocata si eveniment 'grid_double_click' declansat.");
    }
}
*/
/*
void vDbGrid::onClick() {
    ConsoleManager::getInstance().log(L"[vDbGrid::onClick] Click simplu gestionat.");

    int selectedIndex = ListView_GetNextItem(m_handle, -1, LVNI_SELECTED);

    if (selectedIndex != -1) {
        // Obține conținutul primei coloane
        WCHAR buffer[256];
        ListView_GetItemText(m_handle, selectedIndex, 0, buffer, sizeof(buffer) / sizeof(WCHAR));

        // Stochează textul într-un membru al clasei
        m_selectedCellText = buffer;

        // Declanșează evenimentul pentru click simplu
        getEventDispatcher().dispatch("grid_click");

        ConsoleManager::getInstance().log(L"[vDbGrid::onClick] Valoare stocata si eveniment 'grid_click' declansat.");
    }
}
*/
/*
void vDbGrid::onKillFocus() {
    ConsoleManager::getInstance().log(L"[vDbGrid::onKillFocus] Click simplu gestionat.");

    int selectedIndex = ListView_GetNextItem(m_handle, -1, LVNI_SELECTED);

    if (selectedIndex != -1) {
        // Obține conținutul primei coloane
        //WCHAR buffer[256];
        //ListView_GetItemText(m_handle, selectedIndex, 0, buffer, sizeof(buffer) / sizeof(WCHAR));

        // Stochează textul într-un membru al clasei
        m_selectedCellText = L"";

        // Declanșează evenimentul pentru click simplu
        getEventDispatcher().dispatch("grid_click");

        ConsoleManager::getInstance().log(L"[vDbGrid::onKillFocus] Valoare stocata si eveniment 'grid_click' declansat.");
    }
}
*/
/*
void vDbGrid::setColumnLabels(const std::map<std::wstring, std::wstring>& labels) {
    m_columnLabels = labels;
}
*/

void vDbGrid::setColumnWidth(const std::wstring& dbColumnName, int width) {
    m_columnWidths[dbColumnName] = width;
}


unsigned long int vDbGrid::getRowsCount() const {
    return m_rowsCount;
}


/**
 * @brief Returns the index of the currently sorted column.
 * @return The 0-based index of the sorted column, or -1 if no column is sorted.

int vDbGrid::getSortedColumn() const {
    return m_sortedColumn;
}
 */

/**/
/*
 * @brief Checks if the sort direction is ascending.
 * @return true if sorting is ascending, false otherwise.

bool vDbGrid::isSortAscending() const {
    return m_sortAscending;
}
 */
/**
 * @brief Gets the database column name for a given column index.
 * @param columnIndex The 0-based index of the column.
 * @return The database column name as a wstring, or an empty wstring if the index is out of bounds.
 */
std::wstring vDbGrid::getDbColumnName(int columnIndex) const {
    if (columnIndex >= 0 && columnIndex < m_columnNames.size()) {
        return m_columnNames[columnIndex];
    }
    return L"";
}
/*
void vDbGrid::resize() {
    vGrid::resize();
    // Apelăm vControl::resize pentru a muta/redimensiona HWND-ul principal
    /*
    vControl::resize();

    if (!m_handle) return;

    // Iterăm prin coloanele pe care le știm din m_columnNames (ordinea contează)
    for (size_t i = 0; i < m_columnNames.size(); ++i) {
        std::wstring colName = m_columnNames[i];
        int baseWidth = 100; // Valoare default

        if (m_columnWidths.count(colName)) {
            baseWidth = m_columnWidths[colName];
        }

        // Calculăm lățimea adaptată la DPI-ul curent al monitorului
        int scaledWidth = MulDiv(baseWidth, m_currentDpi, 96);
        ListView_SetColumnWidth(m_handle, (int)i, scaledWidth);
    }
    
}
*/

int vDbGrid::getColumnWidthByIndex(int columnIndex) const {
    if (m_handle) {
        // Trimitem mesajul LVM_GETCOLUMNWIDTH direct către controlul Win32 SysListView32
        return ListView_GetColumnWidth(m_handle, columnIndex);
    }
    return 0;
}

void vDbGrid::setColumnLabel(const std::wstring& dbColumnName, const std::wstring& label){
    m_columnLabels[dbColumnName] = label;
    refreshHeaderLabels();
    
}

void vDbGrid::setColumnLabels(const std::map<std::wstring, std::wstring>& labels) {
    m_columnLabels = labels;

    // Dacă grid-ul este deja populat și vrei să schimbi antetele "la cald", 
    // va trebui să apelezi o metodă care face refresh la textul coloanelor.
    if (m_handle && !m_columnNames.empty()) {
        refreshHeaderLabels();
    }
}


void vDbGrid::refreshHeaderLabels() {
    if (!m_handle) return;

    HWND hHeader = ListView_GetHeader(m_handle);
    for (size_t i = 0; i < m_columnNames.size(); ++i) {
        std::wstring dbName = m_columnNames[i];
        if (m_columnLabels.count(dbName)) {
            std::wstring newLabel = m_columnLabels[dbName];

            LVCOLUMN lvc = { 0 };
            lvc.mask = LVCF_TEXT;
            lvc.pszText = const_cast<wchar_t*>(newLabel.c_str());
            ListView_SetColumn(m_handle, i, &lvc);
        }
    }
}

std::wstring vDbGrid::getCellValueByFieldName(int rowIndex, const std::string& fieldName) {
    int colIndex = getColumnIndexByDbName(fieldName);
    if (colIndex == -1) return L"";

    // 1. Întrebăm ListView-ul cât de lung este textul în acea celulă
    // Folosim o dimensiune inițială mare pentru a evita apelurile multiple, 
    // dar suntem pregătiți pentru orice mărime.
    int bufferSize = 1024;
    std::vector<wchar_t> buffer;

    // În Win32, LVM_GETITEMTEXT nu returnează lungimea dacă buffer-ul e prea mic.
    // O strategie sigură este să mărim buffer-ul până când textul încape.

    int actualLen = 0;
    do {
        buffer.resize(bufferSize);
        LVITEMW lvItem = { 0 };
        lvItem.iSubItem = colIndex;
        lvItem.pszText = buffer.data();
        lvItem.cchTextMax = bufferSize;

        actualLen = (int)SendMessage(m_handle, LVM_GETITEMTEXTW, (WPARAM)rowIndex, (LPARAM)&lvItem);

        // Dacă textul umple exact buffer-ul, e posibil să mai fie caractere.
        // Mărim buffer-ul și reîncercăm.
        if (actualLen < bufferSize - 1) {
            break;
        }
        bufferSize *= 2;
    } while (bufferSize < 32768); // Limită de siguranță (32KB per celulă)

    return std::wstring(buffer.data());
}

int vDbGrid::getColumnIndexByDbName(const std::string& fieldName) {
    // Convertim fieldName din std::string în std::wstring pentru comparație, 
    // deoarece m_columnNames este vector de wstring
    std::wstring targetName(fieldName.begin(), fieldName.end());

    for (int i = 0; i < (int)m_columnNames.size(); ++i) {
        if (m_columnNames[i] == targetName) {
            return i;
        }
    }
    return -1; // Nu a fost găsită
}