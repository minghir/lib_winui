#include "vDbSortableGrid.hpp"
#include <algorithm>

vDbSortableGrid::vDbSortableGrid(HINSTANCE hInstance, const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher, dbConnection* db)
    : vDbGrid(hInstance, id, x, y, width, height, dispatcher, db) {}

bool vDbSortableGrid::populate(const std::wstring& query) {
    // Salvăm query-ul de bază (dar verificăm să nu conțină deja un ORDER BY extern)
    m_baseQuery = query;
    return vDbGrid::populate(query);
}

void vDbSortableGrid::onColumnClick(int columnIndex) {
    if (columnIndex < 0 || columnIndex >= (int)m_columnNames.size()) return;

    // 1. Logica de toggle (comutare)
    if (m_sortedColumn == columnIndex) {
        m_sortAscending = !m_sortAscending;
    }
    else {
        m_sortedColumn = columnIndex;
        m_sortAscending = true;
    }

    // 2. Construim noul query cu ORDER BY
    // Folosim numele coloanei din baza de date salvat în m_columnNames
    std::wstring dbColName = m_columnNames[columnIndex];
    std::wstring sortQuery = m_baseQuery + L" ORDER BY " + dbColName + L" " +
        (m_sortAscending ? L"ASC" : L"DESC");

    // 3. Re-populăm grid-ul folosind metoda de bază
    // Notă: vDbGrid::populate va curăța totul și va reface rândurile
    if (vDbGrid::populate(sortQuery)) {
        // 4. Actualizăm vizual săgeata în Header
        updateSortArrow(columnIndex, m_sortAscending);
    }
}

void vDbSortableGrid::updateSortArrow(int columnIndex, bool ascending) {
    HWND hHeader = ListView_GetHeader(m_handle);
    if (!hHeader) return;

    // Resetăm toate coloanele (scoatem săgețile vechi)
    for (int i = 0; i < (int)m_columnNames.size(); ++i) {
        HDITEM hdItem = { 0 };
        hdItem.mask = HDI_FORMAT;
        Header_GetItem(hHeader, i, &hdItem);
        hdItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        if (i == columnIndex) {
            hdItem.fmt |= (ascending ? HDF_SORTUP : HDF_SORTDOWN);
        }
        Header_SetItem(hHeader, i, &hdItem);
    }

    // 2. ACTIVEAZĂ NUANȚA MODERNĂ PE COLOANĂ
    // Acest mesaj îi spune ListView-ului care coloană să fie "evidențiată"
    SendMessage(m_handle, LVM_SETSELECTEDCOLUMN, (WPARAM)columnIndex, 0);
}