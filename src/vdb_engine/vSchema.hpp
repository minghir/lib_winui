#ifndef VSCHEMA_HPP
#define VSCHEMA_HPP

#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "vTable.hpp"

class vSchema {
    std::wstring m_name;
    std::map<std::wstring, std::unique_ptr<vTable>> m_tables;

public:
    vSchema(const std::wstring& name) : m_name(name) {}

    void addTable(std::unique_ptr<vTable> table) {
        std::wstring name = table->getName();
        // Normalizăm cheia la lowercase înainte de stocare
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        m_tables[name] = std::move(table);
    }

    const std::wstring getName() { return m_name; }

    vTable* getTable(const std::wstring& tableName) {
        std::wstring name = tableName;
        // Normalizăm căutarea
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        auto it = m_tables.find(name);
        return (it != m_tables.end()) ? it->second.get() : nullptr;
    }

    const std::map<std::wstring, std::unique_ptr<vTable>>& getTables() const {
        return m_tables; // Presupunând că m_tables este membrul tău privat
    }
};

#endif