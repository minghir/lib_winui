#ifndef VSTORAGEMANAGER_HPP
#define VSTORAGEMANAGER_HPP

#include "vDatabase.hpp"
#include <fstream>
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>


class vStorageManager {
public:
    static bool loadDatabase(vDatabase* db, const std::wstring& filename) {
        std::ifstream inFile{ std::filesystem::path(filename), std::ios::binary };
        
        //std::ifstream inFile(filename, std::ios::binary);
        if (!inFile.is_open()) return false;

        char magic[4];
        inFile.read(magic, 4);
        if (std::string(magic, 4) != "VDB1") return false;

        uint32_t schemaCount = 0;
        if (!inFile.read(reinterpret_cast<char*>(&schemaCount), sizeof(uint32_t))) return false;

        // Protecție: dacă schemaCount este absurd, oprim totul
        if (schemaCount > 1000) return false;

        for (uint32_t i = 0; i < schemaCount; ++i) {
            // Dacă o singură schemă eșuează, tot procesul returnează false
            if (!deserializeSchema(db, inFile)) return false;
        }

        inFile.close();
        return true;
    }

    static bool saveDatabase(vDatabase* db, const std::wstring& filename) {
        // Deschiderea fișierului în mod binar
        
        std::ofstream outFile(std::filesystem::path(filename), std::ios::binary);
        if (!outFile.is_open()) return false;

        // 1. Header Magic
        const char magic[4] = { 'V', 'D', 'B', '1' };
        outFile.write(magic, 4);

        // 2. Număr scheme - folosim referință (&) pentru a evita C2280
        const auto& schemas = db->getSchemas();
        uint32_t schemaCount = static_cast<uint32_t>(schemas.size());
        outFile.write(reinterpret_cast<const char*>(&schemaCount), sizeof(schemaCount));

        for (const auto& [name, schema] : schemas) {
            serializeSchema(schema.get(), outFile);
        }

        outFile.close();
        return true;
    }




    static void inspectDatabase(const std::wstring& filename) {
        auto& console = ConsoleManager::getInstance();
        bool success = true;

        // 1. Verificare mărime fișier (Robust)
        std::error_code ec;
        auto fileSize = std::filesystem::file_size(filename, ec);
        if (ec) {
            LOG_ERROR(L"Nu s-a putut accesa fisierul: " + filename);
            return;
        }
        double sizeInMB = static_cast<double>(fileSize) / (1024.0 * 1024.0);

        // 2. Deschidere stream binar
        //std::ifstream inFile(filename, std::ios::binary);
        std::ifstream inFile{ std::filesystem::path(filename), std::ios::binary };
        if (!inFile.is_open()) {
            LOG_ERROR(L"Eroare la deschiderea stream-ului binar.");
            return;
        }

        // 3. Validare Magic Number
        char magic[4] = { 0 };
        inFile.read(magic, 4);
        if (std::string(magic, 4) != "VDB1") {
            LOG_ERROR(L"CRITICAL: Fisierul nu este format VDB sau header-ul este corupt!");
            return;
        }

        // Header Raport
        console.writeRaw(L"=== VDB Binary Inspection ===", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        console.writeRaw(L"File: " + filename);

        std::wstringstream ssSize;
        ssSize << std::fixed << std::setprecision(2) << sizeInMB;
        console.writeRaw(L"Size: " + ssSize.str() + L" MB", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

        // 4. Citire numar scheme
        uint32_t schemaCount = 0;
        if (!inFile.read(reinterpret_cast<char*>(&schemaCount), sizeof(uint32_t))) success = false;

        console.writeRaw(L"Total Schemas: " + std::to_wstring(schemaCount));
        console.writeRaw(L"---------------------------------", FOREGROUND_BLUE);

        // Buclele de procesare cu fail-fast (&& success)
        for (uint32_t s = 0; s < schemaCount && success; ++s) {
            std::wstring sName = readWString(inFile, success);
            if (!success) break;

            uint32_t tableCount = 0;
            if (!inFile.read(reinterpret_cast<char*>(&tableCount), sizeof(uint32_t))) { success = false; break; }

            console.writeRaw(L"Schema [" + sName + L"] contains " + std::to_wstring(tableCount) + L" tables:");

            for (uint32_t t = 0; t < tableCount && success; ++t) {
                std::wstring tName = readWString(inFile, success);
                if (!success) break;

                uint32_t colCount = 0;
                if (!inFile.read(reinterpret_cast<char*>(&colCount), sizeof(uint32_t))) { success = false; break; }

                // Skip metadate coloane
                for (uint32_t c = 0; c < colCount && success; ++c) {
                    readWString(inFile, success); // Numele coloanei
                    if (!success) break;
                    // Sari peste: Type(int) + Length(int) + Precision(int) + Nullable(bool)
                    inFile.seekg(sizeof(int) * 3 + sizeof(bool), std::ios::cur);
                    if (inFile.fail()) { success = false; break; }
                }

                uint32_t rowCount = 0;
                if (!inFile.read(reinterpret_cast<char*>(&rowCount), sizeof(uint32_t))) { success = false; break; }

                console.writeRaw(L"  -> Table: " + tName + L" (" + std::to_wstring(colCount) + L" cols, " + std::to_wstring(rowCount) + L" rows)");

                // "Consumam" datele fara a le stoca, verificand integritatea la fiecare pas
                for (uint32_t r = 0; r < rowCount && success; ++r) {
                    for (uint32_t c = 0; c < colCount && success; ++c) {
                        if (!skipValue(inFile)) {
                            success = false;
                            break;
                        }
                    }
                }
            }
        }

        if (!success) {
            console.writeRaw(L"---------------------------------", FOREGROUND_RED);
            LOG_ERROR(L"INSPECTIE ABORTATA: Fisierul este corupt sau structura nu se potriveste!");
        }
        else {
            console.writeRaw(L"---------------------------------", FOREGROUND_BLUE);
            console.writeRaw(L"Inspection completed successfully.", FOREGROUND_GREEN);
        }

        inFile.close();
    }

private:
    static void writeWString(const std::wstring& s, std::ofstream& out) {
        uint32_t len = static_cast<uint32_t>(s.length());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        if (len > 0) {
            out.write(reinterpret_cast<const char*>(s.data()), len * sizeof(wchar_t));
        }
    }

    static void serializeSchema(vSchema* schema, std::ofstream& out) {
        writeWString(schema->getName(), out);

        const auto& tables = schema->getTables();
        uint32_t tableCount = static_cast<uint32_t>(tables.size());
        out.write(reinterpret_cast<const char*>(&tableCount), sizeof(tableCount));

        for (const auto& [name, table] : tables) {
            serializeTable(table.get(), out);
        }
    }

    static void serializeTable(vTable* table, std::ofstream& out) {
        writeWString(table->getName(), out);

        // Coloane
        const auto& columns = table->getColumns();
        uint32_t colCount = static_cast<uint32_t>(columns.size());
        out.write(reinterpret_cast<const char*>(&colCount), sizeof(colCount));

        for (const auto& col : columns) {
            writeWString(col.name, out);
            int typeInt = static_cast<int>(col.type);
            out.write(reinterpret_cast<const char*>(&typeInt), sizeof(int));
            out.write(reinterpret_cast<const char*>(&col.nativeLength), sizeof(int));
            out.write(reinterpret_cast<const char*>(&col.nativePrecision), sizeof(int));
            bool nullable = col.isNullable;
            out.write(reinterpret_cast<const char*>(&nullable), sizeof(bool));
        }

        // Date
        const auto& rows = table->getData();
        uint32_t rowCount = static_cast<uint32_t>(rows.size());
        out.write(reinterpret_cast<const char*>(&rowCount), sizeof(rowCount));

        for (const auto& row : rows) {
            for (const auto& cell : row) {
                serializeValue(cell, out);
            }
        }
    }

    static void serializeValue(const vValue& val, std::ofstream& out) {
        uint8_t tag = static_cast<uint8_t>(val.index());
        out.write(reinterpret_cast<const char*>(&tag), sizeof(uint8_t));

        switch (tag) {
        case 0: break; // monostate
        case 1: writeWString(std::get<std::wstring>(val), out); break;
        case 2: {
            int v = std::get<int>(val);
            out.write(reinterpret_cast<const char*>(&v), sizeof(int));
            break;
        }
        case 3: {
            double v = std::get<double>(val);
            out.write(reinterpret_cast<const char*>(&v), sizeof(double));
            break;
        }
        case 4: {
            long long v = std::get<long long>(val);
            out.write(reinterpret_cast<const char*>(&v), sizeof(long long));
            break;
        }
        case 5: {
            bool v = std::get<bool>(val);
            out.write(reinterpret_cast<const char*>(&v), sizeof(bool));
            break;
        }
        }
    }
    
    static std::wstring readWString(std::ifstream& in) {
        uint32_t len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
        if (len == 0) return L"";

        std::wstring s(len, L'\0');
        in.read(reinterpret_cast<char*>(&s[0]), len * sizeof(wchar_t));
        return s;
    }
    
    static std::wstring readWString(std::ifstream& in, bool& success) {
        uint32_t len = 0;
        if (!in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t))) {
            success = false;
            return L"";
        }

        // Protecție: Un nume de tabel/schemă n-ar trebui să aibă 1MB (500k caractere)
        if (len > 500000) {
            success = false;
            return L"";
        }

        if (len == 0) return L"";

        std::wstring s(len, L'\0');
        if (!in.read(reinterpret_cast<char*>(&s[0]), len * sizeof(wchar_t))) {
            success = false;
            return L"";
        }
        return s;
    }

    static bool deserializeSchema(vDatabase* db, std::ifstream& in) {
        bool success = true;
        std::wstring schemaName = readWString(in, success);
        if (!success) return false;

        db->createSchema(schemaName);
        vSchema* schema = db->getSchema(schemaName);

        uint32_t tableCount = 0;
        if (!in.read(reinterpret_cast<char*>(&tableCount), sizeof(uint32_t))) return false;

        // Altă protecție: data_2.vdb avea 16 milioane aici
        if (tableCount > 10000) return false;

        for (uint32_t i = 0; i < tableCount; ++i) {
            if (!deserializeTable(schema, in)) return false;
        }
        return true;
    }

    static bool deserializeTable(vSchema* schema, std::ifstream& in) {
        bool success = true;
        std::wstring tableName = readWString(in, success);
        if (!success) return false;

        auto table = std::make_unique<vTable>(tableName);

        uint32_t colCount = 0;
        if (!in.read(reinterpret_cast<char*>(&colCount), sizeof(uint32_t)) || colCount > 1000) return false;

        for (uint32_t i = 0; i < colCount; ++i) {
            vColumn col;
            col.name = readWString(in, success);
            if (!success) return false;

            in.read(reinterpret_cast<char*>(&col.type), sizeof(int));
            in.read(reinterpret_cast<char*>(&col.nativeLength), sizeof(int));
            in.read(reinterpret_cast<char*>(&col.nativePrecision), sizeof(int));
            in.read(reinterpret_cast<char*>(&col.isNullable), sizeof(bool));

            if (in.fail()) return false;
            table->addColumn(col);
        }

        uint32_t rowCount = 0;
        if (!in.read(reinterpret_cast<char*>(&rowCount), sizeof(uint32_t))) return false;

        for (uint32_t r = 0; r < rowCount; ++r) {
            std::vector<vValue> row;
            for (size_t c = 0; c < colCount; ++c) {
                vValue val = deserializeValue(in, success);
                if (!success) return false;
                row.push_back(val);
            }
            table->addRow(row);
        }

        schema->addTable(std::move(table));
        return true;
    }

    static vValue deserializeValue(std::ifstream& in, bool& success) {
        uint8_t tag = 0;
        if (!in.read(reinterpret_cast<char*>(&tag), sizeof(uint8_t))) {
            success = false;
            return std::monostate{};
        }

        switch (tag) {
        case 0: return std::monostate{};
        case 1: return readWString(in, success);
        case 2: {
            int v;
            in.read(reinterpret_cast<char*>(&v), sizeof(int));
            return v;
        }
        case 3: {
            double v;
            in.read(reinterpret_cast<char*>(&v), sizeof(double));
            return v;
        }
        case 4: {
            long long v;
            in.read(reinterpret_cast<char*>(&v), sizeof(long long));
            return v;
        }
        case 5: {
            bool v;
            in.read(reinterpret_cast<char*>(&v), sizeof(bool));
            return v;
        }
        default:
            success = false;
            return std::monostate{};
        }
    }

    // Helper pentru a sari peste o valoare vValue fara a aloca memorie
    static bool skipValue(std::ifstream& in) {
        uint8_t tag = 0;
        if (!in.read(reinterpret_cast<char*>(&tag), sizeof(uint8_t))) return false;

        switch (tag) {
        case 0: // monostate (null)
            return true;
        case 1: { // wstring
            uint32_t len = 0;
            if (!in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t))) return false;
            in.seekg(len * sizeof(wchar_t), std::ios::cur);
            return true;
        }
        case 2: // int
            in.seekg(sizeof(int), std::ios::cur);
            return true;
        case 3: // double
            in.seekg(sizeof(double), std::ios::cur);
            return true;
        case 4: // long long
            in.seekg(sizeof(long long), std::ios::cur);
            return true;
        case 5: in.seekg(sizeof(bool), std::ios::cur); 
            return true;
        default:
            return false; // Tag necunoscut = fisier corupt
        }
    }
};

#endif