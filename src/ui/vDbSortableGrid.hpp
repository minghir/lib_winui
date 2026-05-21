#ifndef VDBSORTABLEGRID_HPP
#define VDBSORTABLEGRID_HPP

#pragma once

#include "vDbGrid.hpp"

class vDbSortableGrid : public vDbGrid {
public:
    vDbSortableGrid(
        HINSTANCE hInstance,
        const std::string& id,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher,
        dbConnection* db
    );

    // Reținem ultimul query pentru a-i putea aplica clauza ORDER BY
    bool populate(const std::wstring& query) override;

protected:
    // Suprascriem hook-ul de click pe coloană definit în vGrid
    void onColumnClick(int columnIndex) override;

private:
    std::wstring m_baseQuery;   // Query-ul original fără ORDER BY
    int m_sortedColumn = -1;    // Indexul coloanei sortate curent
    bool m_sortAscending = true; // Direcția sortării

    /** Adaugă micul triunghi de sortare în antetul coloanei Win32 */
    void updateSortArrow(int columnIndex, bool ascending);
};

#endif