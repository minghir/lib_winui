#include "vDataLoader.hpp"
#include "../ui/ConsoleManager.hpp"

bool vDataLoader::loadIntoTable(dbConnection* conn, const std::wstring& tableName, const std::wstring& schemaName) {
    std::string stm = "import_stm";
    if (!conn->execQuery(L"SELECT * FROM " + tableName, stm)) return false;

    auto virtualTable = std::make_unique<vTable>(tableName);

    // 1. Preluăm metadatele complete (Nume, Tip Universal, Lungime, Precizie)
    const auto& columnsMeta = conn->getColumnsInfo(stm);

    // 2. Construim structura tabelului folosind noile metadate
    for (const auto& colInfo : columnsMeta) {
        vDataType eType = mapToEngineType(colInfo.type);

        // Folosim varianta addColumn care acceptă lungimea și precizia
        vColumn newCol;
        newCol.name = colInfo.name;
        newCol.type = mapToEngineType(colInfo.type);
        newCol.nativeLength = colInfo.length;
        newCol.nativePrecision = colInfo.precision;
        newCol.isNullable = colInfo.isNullable; 

        virtualTable->addColumn(newCol);
    }

    // 3. Încărcarea rândurilor (Rămâne neschimbată, dar folosim vTable pentru tipuri)
    while (conn->fetchNextRow(stm)) {
        std::vector<std::wstring> rawRow = conn->fetchRow(stm);
        std::vector<vValue> typedRow;

        const auto& tableCols = virtualTable->getColumns();
        for (size_t i = 0; i < rawRow.size(); ++i) {
            typedRow.push_back(convertToTypedValue(rawRow[i], tableCols[i].type));
        }
        virtualTable->addRow(typedRow);
    }

    m_engine->getDB()->addTableToSchema(schemaName, std::move(virtualTable));
    return true;
}

vValue vDataLoader::convertToTypedValue(const std::wstring& raw, vDataType type) {
    if (raw.empty()) {
        return std::monostate{};
    }

    try {
        switch (type) {
        case vDataType::Boolean:
            // Interpretăm "1" ca true, restul ca false
            // Această logică se potrivește cu ce am pus în dbfConnection::fetchFieldByNumber
            return (raw == L"1");

        case vDataType::Integer:
            try {
                return std::stoll(raw);
            }
            catch (...) {
                return std::monostate{};
            }

        case vDataType::Double:
            try {
                return std::stod(raw);
            }
            catch (...) {
                return std::monostate{};
            }

        case vDataType::Text:
        case vDataType::Date:
        default:
            return raw;
        }
    }
    catch (...) {
        return raw;
    }
}

vDataType vDataLoader::mapToEngineType(vNativeDataType universalType) {
    switch (universalType) {
    case vNativeDataType::V_INTEGER:
    case vNativeDataType::V_BIGINT:
        return vDataType::Integer;

    case vNativeDataType::V_DOUBLE:
        return vDataType::Double;

    case vNativeDataType::V_DATE:
        return vDataType::Date;

    case vNativeDataType::V_TEXT:
        return vDataType::Text;

    case vNativeDataType::V_BOOLEAN:
        // Aici mapăm tipul Logical din DBF către tipul Boolean din Engine
        return vDataType::Boolean;

    case vNativeDataType::V_NULL:
    default:
        return vDataType::Text;
    }
}
    
    bool vDataLoader::importExternalData(const std::wstring & type, const std::wstring & path, const std::wstring & target) {
        if (type != L"dbf") {
            LOG_ERROR(L"Tip de import nesuportat: " + type);
            return false;
        }

        // 1. Parsare target (schema.table)
        std::wstring schemaName = L"public";
        std::wstring tableName;

        size_t dotPos = target.find(L'.');
        if (dotPos != std::wstring::npos) {
            schemaName = target.substr(0, dotPos);
            tableName = target.substr(dotPos + 1);
        }
        else {
            tableName = target;
        }

        // Fallback pe numele fișierului dacă tabelul nu e specificat
        if (tableName.empty()) {
            tableName = std::filesystem::path(path).stem().wstring();
        }

        // 2. Logica specifică pentru DBF
        auto conn = std::make_unique<dbfConnection>("DBF_NATIVE", path);
        if (!conn->openDatabase()) {
            LOG_ERROR(L"Nu s-a putut deschide sursa DBF: " + path);
            return false;
        }

        // 3. Apelăm metoda de încărcare pe care o avem deja
        LOG_INFO(L"Incepere proces de incarcare in " + schemaName + L"." + tableName);
        bool success = loadIntoTable(conn.get(), tableName, schemaName);

        conn->closeDatabase();

        if (success) {
            LOG_SUCCESS(L"Importul datelor a fost finalizat cu succes.");
        }

        return success;
    }
    