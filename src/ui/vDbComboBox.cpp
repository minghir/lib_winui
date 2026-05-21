#include "vDbComboBox.hpp"
#include "..\stringUtils.hpp"
#include "ConsoleManager.hpp"
#include <sstream>

// Constructorul vDbComboBox apelează constructorul clasei de bază (vComboBox)
// și inițializează pointerul la baza de date.
vDbComboBox::vDbComboBox(
    HINSTANCE hInstance,
    const std::string& id,
    int x, int y, int width, int height,
    EventDispatcher& dispatcher,
    dbConnection* db
)
    : vComboBox(hInstance, id, x, y, width, height, dispatcher, 150),
    m_db(db) {
   
}

// Destructor.
vDbComboBox::~vDbComboBox() {
    // Nu este nevoie să ștergi m_db, deoarece proprietatea este deținută de vApp/vDbApp.
}

bool vDbComboBox::populate(const std::wstring& query, const std::wstring& selectValue) {
    m_targetQuery = query;

    if (!m_db) {
        LOG_ERROR(L"vDbComboBox: Nu pot popula fără o conexiune validă!");
        return false;
    }

    if (!m_db || !m_db->isConnected()) {
        ConsoleManager::getInstance().log(L"[ERROR] vDbComboBox: DB neconectat.");
        return false;
    }

    clearItems();
    m_itemValues.clear();

    if (!m_db->execQuery(query)) {
        ConsoleManager::getInstance().log(L"[ERROR] SQL Fail: " + m_db->getError());
        return false;
    }

    // Pasul 1: Popularea listei
    while (m_db->fetchNextRow()) {
        std::vector<std::wstring> row = m_db->fetchRow();
        if (row.size() >= 2) {
            // row[0] este ID-ul, row[1] este Textul
            this->addItem(row[1], 0);
            m_itemValues.push_back(row[0]);
        }
    }
    if (m_itemValues.empty()) return true;

    if (!selectValue.empty()) {
        setSelectedByStringValue(selectValue);
    }
    else {
        setSelectedIndex(0);
    }

    return true;
}

/*
void vDbComboBox::setSelectedByStringValue(const std::wstring& targetValue) {
    
    if (!m_handle) return;
    
    for (size_t i = 0; i < m_itemValues.size(); i++) {
        if (m_itemValues[i] == targetValue) {
            
            setSelectedIndex((int)i);
            ::InvalidateRect(m_handle, NULL, TRUE);
            return;
        }
    }
    // Dacă nu găsește valoarea, putem lăsa neselectat
    setSelectedIndex(-1);
}
*/

void vDbComboBox::setSelectedByStringValue(const std::wstring& targetValue) {
    // 1. Verifică handle-ul (Garda de siguranță)
    if (!m_handle || !::IsWindow(m_handle)) return;

    // 2. Trimite un mesaj de blocare a redraw-ului dacă sunt mulți itemi (opțional)
    // ::SendMessage(m_handle, WM_SETREDRAW, FALSE, 0);

    bool found = false;
    for (size_t i = 0; i < m_itemValues.size(); i++) {
        // Curăță spațiile invizibile (trim) dacă datele vin din DB fix-length (char(10))
        if (wstr_trim(m_itemValues[i]) == wstr_trim(targetValue)) {
            setSelectedIndex((int)i);
            found = true;
            break;
        }
    }

    if (!found) {
        setSelectedIndex(-1);
    }

    // ::SendMessage(m_handle, WM_SETREDRAW, TRUE, 0);
    ::InvalidateRect(m_handle, NULL, TRUE);
}

std::wstring vDbComboBox::getSelectedStringValue() const {
    int index = getSelectedIndex();
    if (index >= 0 && index < (int)m_itemValues.size()) {
        return m_itemValues[index];
    }
    return L"";
}