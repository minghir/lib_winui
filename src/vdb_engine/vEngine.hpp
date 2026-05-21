#ifndef VENGINE_HPP
#define VENGINE_HPP

#pragma once


#include "vDatabase.hpp"
//#include "vDataLoader.hpp"
#include "vStorageManager.hpp"
#include "../sql/SqlQueryParser.hpp"
#include "../ui/ConsoleManager.hpp"

#include <filesystem>

namespace fs = std::filesystem;


// Structură pentru a returna rezultatele într-un mod organizat
struct QueryResult {
    bool success = false;
    std::wstring errorMessage;

    // Structura tabelului rezultat (Metadata)
    std::vector<vColumn> columns;
    std::vector<std::wstring> aliases;

    // Datele propriu-zise
    //std::vector<std::vector<std::wstring>> data;
    std::vector<std::vector<vValue>> data;

    // Statistici utile
    size_t rowCount = 0;
    double executionTimeMs = 0.0;

    void clear() {
        success = false;
        errorMessage.clear();
        columns.clear();
        data.clear();
        rowCount = 0;
    }
};

class vEngine {
private:
    std::unique_ptr<vDatabase> m_db;
    bool m_isRunning;

public:
    vEngine() : m_isRunning(false) {
        m_db = std::make_unique<vDatabase>();
    }

    // 1. INIT: Încarcă datele (ex: din DBF)
    bool init() {
        LOG_INFO(L"vEngine: Se inițializează motorul de date...");
        // Aici poți crea schemele default sau încărca fișierele de configurare
        m_isRunning = true;
        m_db->addSchema(L"admin");
        auto usersTable = std::make_unique<vTable>(L"users");
        usersTable->addColumn(L"ID", vDataType::Integer);
        usersTable->addColumn(L"USERNAME", vDataType::Text);
        usersTable->addColumn(L"PASSWORD", vDataType::Text);
        usersTable->addRow({ 1 ,L"admin",L"admin"});

        m_db->addTableToSchema(L"admin", std::move(usersTable));
        return true;
    }
    
    /*
    QueryResult executeQuery(const std::wstring& sql) {
        QueryResult result;
        auto startTime = std::chrono::high_resolution_clock::now();

        // 1. Parsare
        SqlQueryParser parser;
        if (!parser.parse(sql)) {
            result.errorMessage = L"SQL Syntax Error near: " + sql.substr(0, 10);
            return result;
        }

        LOG_DEBUG(L"Query Parser a extras tabelul: '" + parser.getTableName() + L"'");

        // 2. Identificare Tabel
        vTable* table = m_db->resolveTable(parser.getTableName());
        if (!table) {
            result.errorMessage = L"Table not found: " + parser.getTableName();
            return result;
        }

        // 3. Selecție coloane (pentru SELECT *)
        std::vector<int> projectionIndices;
        bool selectAll = false;

        for (const auto& sqlCol : parser.getColumns()) {
            if (sqlCol.column == "*") {
                selectAll = true;
                break;
            }
            int idx = table->getColumnIndex(std::wstring(sqlCol.column.begin(), sqlCol.column.end()));
            if (idx != -1) {
                projectionIndices.push_back(idx);
                result.columns.push_back(table->getColumns()[idx]);
            }
        }

        if (selectAll) {
            result.columns = table->getColumns();
            result.data = table->getData();
        }
        else {
            // Proiecție: construim noul set de date doar cu coloanele cerute
            for (const auto& fullRow : table->getData()) {
                std::vector<std::wstring> projectedRow;
                for (int idx : projectionIndices) {
                    projectedRow.push_back(fullRow[idx]);
                }
                result.data.push_back(projectedRow);
            }
        }

        // 4. Filtrare (Logica de WHERE va veni aici)
      
        result.rowCount = result.data.size();
        result.success = true;

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        return result;
    }
    */

    QueryResult executeQuery(const std::wstring& sql) {
        QueryResult result;
        auto startTime = std::chrono::high_resolution_clock::now();

        // 1. Parsare
        SqlQueryParser parser(L"");
        if (!parser.parse(sql)) {
            result.errorMessage = L"SQL Syntax Error near: " + sql.substr(0, 10);
            return result;
        }

        // 2. Identificare Tabel
        vTable* table = m_db->resolveTable(parser.getTableName());
        if (!table) {
            result.errorMessage = L"Table not found: " + parser.getTableName();
            return result;
        }

        // 3. Selecție coloane
        std::vector<int> projectionIndices;
        bool selectAll = false;

        for (const auto& sqlCol : parser.getColumns()) {
            if (sqlCol.alias == L"*") {
                selectAll = true;
                break;
            }

            // Conversie din string (parser) în wstring pentru index
            //std::wstring colNameW(sqlCol.column.begin(), sqlCol.column.end());
            int idx = table->getColumnIndex(sqlCol.alias);

            if (idx != -1) {
                projectionIndices.push_back(idx);
                result.columns.push_back(table->getColumns()[idx]);

                result.aliases.clear();
                for (const auto& col : result.columns) {
                    result.aliases.push_back(col.name);
                }
                //result.aliases.push_back(sqlCol.alias);
            }
            else {
                result.errorMessage = L"Unknown column name: " + sqlCol.alias;
                return result;
            }
        }

        // 4. Extracție Date (Proiecție)
        if (selectAll) {
            result.columns = table->getColumns();
            result.aliases.clear();
            for (const auto& col : result.columns) {
                result.aliases.push_back(col.name);
            }
            result.data = table->getData(); // Copiază vectorul de vector<vValue>
        }
        else {
            // Construim noul set de date tipizat
            for (const auto& fullRow : table->getData()) {
                std::vector<vValue> projectedRow; // Schimbat din std::wstring
                for (int idx : projectionIndices) {
                    projectedRow.push_back(fullRow[idx]); // Transferăm variant-ul direct
                }
                result.data.push_back(projectedRow);
            }
        }

        result.rowCount = result.data.size();
        result.success = true;

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        return result;
    }
    // 3. SHUTDOWN: Eliberare resurse
    void shutdown() {
        LOG_INFO(L"vEngine: Oprire sistem...");
        m_isRunning = false;
        // Curățenie dacă e cazul
    }

    vDatabase* getDB() { return m_db.get(); }

    QueryResult getSchemaCatalog() {
        QueryResult result;
        auto startTime = std::chrono::high_resolution_clock::now();

        result.columns = { {L"Schema"}, {L"Table Name"}, {L"Rows"} };
        for (size_t i = 0; i < result.columns.size(); ++i) {
            result.aliases.push_back(result.columns[i].name);
        }

        const auto& schemas = m_db->getSchemas();

        for (auto const& [schemaName, schemaPtr] : schemas) {
            const auto& tables = schemaPtr->getTables();

            for (auto const& [tableName, tablePtr] : tables) {
                // MODIFICARE: Folosim vValue în loc de wstring pentru rând
                std::vector<vValue> row;

                // Conversia de la wstring la vValue se face implicit aici
                row.push_back(vValue(schemaName));
                row.push_back(vValue(tableName));

                // Pentru numărul de rânduri, e mai bine să îl stocăm ca număr, nu string
                row.push_back(vValue(static_cast<long long>(tablePtr->getData().size())));

                result.data.push_back(row); // Acum argumentele se potrivesc
            }
        }

        result.success = true;
        result.rowCount = (int)result.data.size();

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        return result;
    }

    QueryResult describeTable(const std::wstring& tableName) {
        QueryResult result;
        vTable* table = m_db->resolveTable(tableName);

        if (!table) {
            result.success = false;
            result.errorMessage = L"Table not found: " + tableName;
            return result;
        }

        result.columns = {
            {L"Index", vDataType::Integer},
            {L"Column Name", vDataType::Text},
            {L"Type", vDataType::Text},
            {L"Length", vDataType::Text},      // Am schimbat în Text pentru a afișa "MAX"
            {L"Precision", vDataType::Integer},
            {L"Nullable", vDataType::Text}     // "YES" sau "NO"
        };

        for (size_t i = 0; i < result.columns.size(); ++i) {
            result.aliases.push_back(result.columns[i].name);
        }

        const auto& tableCols = table->getColumns();
        for (size_t i = 0; i < tableCols.size(); ++i) {
            std::vector<vValue> row;

            row.push_back((int)i);
            row.push_back(tableCols[i].name);

            // Mapare Tip
            std::wstring cType;
            switch (tableCols[i].type) {
            case vDataType::Date:    cType = L"DATE"; break;
            case vDataType::Double:  cType = L"DOUBLE"; break;
            case vDataType::Integer: cType = L"INTEGER"; break;
            case vDataType::Text:    cType = L"TEXT"; break;
            case vDataType::Boolean: cType = L"BOOLEAN"; break; // <--- Adaugă linia asta
            default:                 cType = L"UNKNOWN"; break;
            }
            row.push_back(cType);

            // LOGICA PENTRU LUNGIME (0 -> MAX)
            if (tableCols[i].nativeLength <= 0) {
                row.push_back(L"MAX");
            }
            else {
                row.push_back(std::to_wstring(tableCols[i].nativeLength));
            }

            row.push_back(tableCols[i].nativePrecision);

            // LOGICA PENTRU NULLABLE
            row.push_back(tableCols[i].isNullable ? L"YES" : L"NO");

            result.data.push_back(row);
        }

        result.success = true;
        result.rowCount = (int)result.data.size();
        return result;
    }

    bool saveState(const std::wstring& filePath) {
        try {
            LOG_INFO(L"Initiating binary serialization to: " + filePath);
            // Transmitem baza de date (m_db) managerului de stocare
            if (vStorageManager::saveDatabase(m_db.get(), filePath)) {
                return true;
            }
        }
        catch (const std::exception& e) {
            std::string err(e.what());
            LOG_ERROR(L"Serialization exception: " + std::wstring(err.begin(), err.end()));
        }
        return false;
    }

    bool loadState(const std::wstring& filePath) {
        LOG_INFO(L"Restoring database state...");

        // 1. Cream o instanta noua (asta sterge tot ce era inainte in m_db)
        auto newDb = std::make_unique<vDatabase>();

        // 2. Incarcam datele din fisier in aceasta instanta noua
        // Daca incarcarea esueaza, nu am pierdut baza de date veche inca
        if (vStorageManager::loadDatabase(newDb.get(), filePath)) {

            // 3. Succes! Inlocuim baza de date veche cu cea incarcata
            m_db = std::move(newDb);

            LOG_SUCCESS(L"Database state restored from " + filePath);
            return true;
        }

        LOG_ERROR(L"Failed to restore database state. Keeping current session data.");
        return false;
    }

    void inspectState(const std::wstring& filePath) {
        // Verificăm dacă fișierul există înainte de a începe
        if (!std::filesystem::exists(filePath)) {
            LOG_ERROR(L"File not found: " + filePath);
            return;
        }

        // Delegăm către managerul de stocare
        vStorageManager::inspectDatabase(filePath);
    }
    /*
    bool importData(const std::wstring& type, const std::wstring& path, const std::wstring& target) {
        // Tot ce face Engine-ul este să instanțieze Loader-ul și să îi dea comanda
        vDataLoader loader(this);
        return loader.importExternalData(type, path, target);
    }
    */
};

#endif