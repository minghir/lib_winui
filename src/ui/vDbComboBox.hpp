#ifndef VDBCOMBOBOX_HPP
#define VDBCOMBOBOX_HPP

#pragma once

#include "vComboBox.hpp"
#include "..\dbConnection.hpp"
#include <string>
#include <vector>

class vDbComboBox : public vComboBox {

  
public:
    // Constructor. Primește un pointer la conexiunea la baza de date.
    explicit vDbComboBox(
        HINSTANCE hInstance,
        const std::string& id,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher,
        dbConnection* db
    );

    // Destructor.
    virtual ~vDbComboBox();

    // Metodă pentru a popula ComboBox-ul dintr-un query SQL.
    // Query-ul trebuie să returneze cel puțin o coloană.
    // Dacă query-ul returnează două coloane, prima este folosită pentru textul elementului, iar a doua pentru datele asociate (LPARAM).
    bool populate(const std::wstring& query, const std::wstring& selectValue);

    bool populate(const std::wstring& selectValue = L"") {
        return populate(m_targetQuery, selectValue);
    }
    void setSelectedByStringValue(const std::wstring& targetValue);

    void setTargetQuery(std::wstring query) {
        m_targetQuery = query;
    }

    const std::wstring getTargetQuery() {
        return m_targetQuery;
    }

    void setDbConnection(dbConnection* db) {
        m_db = db;
    }

    //LPARAM getSelectedValue() const;

    std::wstring getSelectedStringValue() const;

    /*
    void setText(const std::wstring& text) override {
        // În loc să scrii textul pur și simplu (ceea ce un Combo nu știe să facă direct),
        // folosim valoarea (ID-ul) venită din DB pentru a selecta elementul corect.
        this->setSelectedByStringValue(text);
    }
    */

    void setText(const std::wstring& text) override {
        // Verifică dacă textul nou este diferit de cel curent pentru a evita buclele
        if (getSelectedStringValue() == text) return;

        // IMPORTANT: Verifică dacă ai itemi. Dacă setText e chemat înainte de populate, 
        // m_itemValues e gol și nu are ce selecta.
        if (m_itemValues.empty()) {
            LOG_WARNING(L"vDbComboBox [" + str_to_wstr(m_id) + L"]: setText apelat pe un combo nepopulat.");
            return;
        }

        this->setSelectedByStringValue(text);
    }

private:
    dbConnection* m_db; // Pointerul la conexiunea la baza de date.
    std::wstring m_targetQuery = L"";

    std::vector<std::wstring> m_itemValues;
};

#endif // VDBCOMBOBOX_HPP