#ifndef VTABLE_HPP
#define VTABLE_HPP

#pragma once

#include <variant>
#include <string>
#include <vector>
#include <map>

using vValue = std::variant<std::monostate, std::wstring, int, double, long long, bool>;

// Tipuri de date suportate de motorul tău SQL
enum class vDataType {
    Text,
    Integer,
    Double,
    Date,
    Boolean
};

// Definiția unei coloane (Nume + Tip)
struct vColumn {
    std::wstring name;
    vDataType type;
    int nativeLength = 0;   // Dimensiunea din sursa originală (ex: DBF fieldLength)
    int nativePrecision = 0; // Pentru numere (zecimale)
    bool isNullable = true; // Implicit, coloanele permit NULL
    std::wstring description;

};

class vTable {
private:
    std::wstring m_name;
    std::vector<vColumn> m_columns;
    // Mapăm numele coloanei la indexul ei (0, 1, 2...) pentru viteză
    std::map<std::wstring, size_t> m_columnIndex;

    // Stocarea datelor: un vector de rânduri, fiecare rând fiind un vector de string-uri
    //std::vector<std::vector<std::wstring>> m_rows;
    std::vector<std::vector<vValue>> m_rows;
public:
    vTable(const std::wstring& name) : m_name(name) {}

    ~vTable() = default;

    const std::wstring& getName() const { return m_name; }

    // Configurarea structurii tabelului
    /*
    void addColumn(const std::wstring& name, vDataType type) {
        m_columnIndex[name] = m_columns.size();
        m_columns.push_back({ name, type });
    }
    */
    // Opțiunea A: Adăugare prin parametri (simplu de apelat)
    void addColumn(const std::wstring& name, vDataType type, int length = 0, int precision = 0) {
        m_columnIndex[name] = m_columns.size();
        m_columns.push_back({ name, type, length, precision, true, L"" });
    }

    // Opțiunea B: Adăugare prin structură (curat pentru obiecte complexe)
    void addColumn(const vColumn& col) {
        m_columnIndex[col.name] = m_columns.size();
        m_columns.push_back(col);
    }

    // Inserarea datelor
    /*
    void addRow(const std::vector<std::wstring>& rowData) {
        // Opțional: poți valida aici dacă rowData.size() == m_columns.size()
        m_rows.push_back(rowData);
    }
    */
    void addRow(const std::vector<vValue>& rowData) {
        m_rows.push_back(rowData);
    }

    // Acces pentru Motorul de Query (Executor)
    const std::vector<vColumn>& getColumns() const { return m_columns; }
    //const std::vector<std::vector<std::wstring>>& getData() const { return m_rows; }
    const std::vector<std::vector<vValue>>& getData() const { return m_rows; }

    int getColumnIndex(const std::wstring& colName) const {
        auto it = m_columnIndex.find(colName);
        if (it != m_columnIndex.end()) {
            return (int)it->second;
        }
        return -1; // Coloana nu există
    }

    size_t getRowCount() const { return m_rows.size(); }
};

#endif