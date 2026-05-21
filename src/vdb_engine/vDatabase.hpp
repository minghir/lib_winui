#ifndef VDATABASE_HPP
#define VDATABASE_HPP

#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <algorithm>


#include "vSchema.hpp"
#include "../ui/ConsoleManager.hpp"
class vDatabase {
private:
    std::map<std::wstring, std::unique_ptr<vSchema>> m_schemas;
    std::wstring m_defaultSchemaName;

public:
    vDatabase() : m_defaultSchemaName(L"public") {
        // Creăm schema default la inițializare
        m_schemas[m_defaultSchemaName] = std::make_unique<vSchema>(m_defaultSchemaName);
    }

    void setDefaultSchema(const std::wstring& name) {
        if (m_schemas.count(name)) {
            m_defaultSchemaName = name;
        }
    }

    void addSchema(const std::wstring& name) {
        std::wstring sName = name;
        std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);

        if (m_schemas.find(sName) == m_schemas.end()) {
            m_schemas[sName] = std::make_unique<vSchema>(sName);
        }
    }

    void createSchema(const std::wstring& name) {
        addSchema(name);
    }

    vSchema* getSchema(const std::wstring& name) {
        std::wstring sName = name;
        std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);

        if (m_schemas.count(sName)) {
            return m_schemas[sName].get();
        }
        return nullptr;
    }

    vTable* resolveTable(const std::wstring& rawName) {
        std::wstring name = rawName;
        // Transformăm totul în lowercase pentru consistență la căutare
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        size_t dotPos = name.find(L'.');
        if (dotPos != std::wstring::npos) {
            std::wstring schemaPart = name.substr(0, dotPos);
            std::wstring tablePart = name.substr(dotPos + 1);

            if (m_schemas.count(schemaPart)) {
                return m_schemas[schemaPart]->getTable(tablePart);
            }
        }
        else {
            if (m_schemas.count(m_defaultSchemaName)) {
                return m_schemas[m_defaultSchemaName]->getTable(name);
            }
        }
        return nullptr;
    }

    void addTableToSchema(const std::wstring& schemaName, std::unique_ptr<vTable> table) {
        std::wstring sName = schemaName;
        std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);

        if (m_schemas.count(sName)) {
            m_schemas[sName]->addTable(std::move(table));
        }
        else {
            // Log-ul care ți-a apărut ție:
            LOG_ERROR(L"Schema negasita: " + sName);
        }
    }

    const std::map<std::wstring, std::unique_ptr<vSchema>>& getSchemas() const {
        return m_schemas;
    }

    void debugDump() {
        LOG_DEBUG(L"--- Structura VDB ---");
        for (auto const& [sName, schema] : m_schemas) {
            LOG_DEBUG(L"Schema: " + sName);
            // Dacă adaugi o metodă listTables() în vSchema, apeleaz-o aici
        }
    }

    void clear() {
        m_schemas.clear();
        // Recreăm schema default pentru a lăsa baza într-o stare validă
        m_schemas[L"public"] = std::make_unique<vSchema>(L"public");
        m_defaultSchemaName = L"public";
        LOG_DEBUG(L"Database cleared (all schemas dropped).");
    }
};

#endif