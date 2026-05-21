#ifndef VDATALOADER_HPP
#define VDATALOADER_HPP

#pragma once

#include "../dbConnection.hpp"
#include "../dbfConnection.hpp"
#include "vEngine.hpp"
#include <memory>

class vDataLoader {
private:
    vEngine* m_engine;

    // Metodă privată pentru a traduce tipurile de date (simplificată)
    vDataType mapColumnType(const std::string& connectionType, const std::wstring& fieldTypeStr);
    vDataType mapToEngineType(vNativeDataType universalType);
public:
    vDataLoader(vEngine* engine) : m_engine(engine) {}
    vValue convertToTypedValue(const std::wstring& raw, vDataType type);
    // Metoda principală care încarcă datele dintr-o conexiune într-un tabel virtual
    bool loadIntoTable(dbConnection* conn, const std::wstring& tableName, const std::wstring& schemaName = L"public");
    bool importExternalData(const std::wstring& type, const std::wstring& path, const std::wstring& target);
    
};

#endif