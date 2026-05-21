#ifndef VDBGRID_HPP
#define VDBGRID_HPP

#pragma once

#include "vGrid.hpp"
#include "..\dbConnection.hpp"
#include <string>
#include <map>

// Adaugă o dependență pentru a folosi imaginile în antetul coloanei
#include <commctrl.h>

class vDbGrid : public vGrid {
public:
    // Constructor
    explicit vDbGrid(
        HINSTANCE hInstance,
        const std::string& id,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher,
        dbConnection* db
    );

    // Destructor
    virtual ~vDbGrid() = default;

    
    virtual bool populate(const std::wstring& query);
    void clear();
   
    const std::vector<std::wstring>& getColumnNames() const;
    const std::map<std::wstring, int>& getColumnWidths() const;
    unsigned long int getRowsCount() const;

    void setColumnLabels(const std::map<std::wstring, std::wstring>& labels);
    void setColumnLabel(const std::wstring& dbColumnName, const std::wstring& label);
    void setColumnWidth(const std::wstring& dbColumnName, int width);
   // void setUniqueIdField(std::string str) { m_uniqueIdField = str; }
   // std::string getUniqueIdField() { return m_uniqueIdField; }

    std::wstring getQuery() { return m_query; }

    std::wstring getDbColumnName(int columnIndex) const;

    int getColumnWidthByIndex(int columnIndex) const;

    std::wstring getCellValueByFieldName(int rowIndex, const std::string& fieldName);

    std::wstring getColumnName(int index) const {
        if (index >= 0 && index < (int)m_columnNames.size()) {
            return m_columnNames[index];
        }
        return L"Coloana " + std::to_wstring(index);
    }

    int getColumnCount() const {
        return (int)m_columnNames.size();
    }

    std::wstring getColumnFieldName(int index) const {
        if (index >= 0 && index < (int)m_columnNames.size()) {
            return m_columnNames[index];
        }
        return L"";
    }

    std::wstring getCellValue(int rowIndex, int colIndex) const {
        // Aici apelezi metoda ta existentă de preluare a textului din grid (vGrid)
        // Probabil se numește getItemText sau similar în clasa de bază vGrid
        return this->getCellText(rowIndex, colIndex);
    }

protected:
    dbConnection* m_db;

    std::wstring m_query;
   // std::string m_uniqueIdField;
    
    std::vector<std::wstring> m_columnNames;
    std::map<std::wstring, std::wstring> m_columnLabels; // Rămâne membru privat
    std::map<std::wstring, int> m_columnWidths; // Numele coloanei -> Lățimea
    unsigned long int m_rowsCount = 0;

    int getColumnIndexByDbName(const std::string& fieldName);
    void refreshHeaderLabels();
};

#endif // VDBGRID_HPP


