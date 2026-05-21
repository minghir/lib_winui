#pragma once
#include "vWindow.hpp"
#include "vButton.hpp"
#include "vDbFormPanel.hpp"
#include "../dbConnection.hpp"
#include "../stringUtils.hpp"

enum class EditMode {
    Insert,
    Update,
    Delete
};


class vDbEditDialog : public vWindow{

protected:
	dbConnection* m_db;
	std::string m_uniqueIdField;
	std::wstring m_uniqueIdValue;
    std::wstring m_query;
    std::wstring m_editTable;
    bool m_actionSuccessful = false;

    EditMode m_mode;

    
    vPanel* m_actionPanel = nullptr;
    vButton* m_actionButton = nullptr;
protected:
    vDbFormPanel* m_DbFormPanel = nullptr;
public:
	explicit vDbEditDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher, dbConnection* db, EditMode mode = EditMode::Update);
    // --- Setters ---

    // Setează ce coloană este considerată cheie primară
    void setUniqueIdField(const std::string& fieldName) { m_uniqueIdField = fieldName; }

    // Setează ID-ul înregistrării pe care o edităm în acest moment
    void setUniqueIdValue(const std::wstring& value) { m_uniqueIdValue = value; }

    // Setează conexiunea la baza de date (dacă vrei să o schimbi la runtime)
    void setDbConnection(dbConnection* db) { m_db = db; }

    void setQuery(const std::wstring& query) { m_query = query; }
    void setTableToEdit(const std::wstring& table) { m_editTable = table; }


    void setMode(EditMode mode) {
        m_actionSuccessful = false;
        m_mode = mode;
        updateUIForMode(); // Actualizăm titlul și butoanele când schimbăm modul
        //populateControls();
    }

     // --- Getters ---

    const std::string& getUniqueIdField() const { return m_uniqueIdField; }
    const std::wstring& getUniqueIdValue() const { return m_uniqueIdValue; }
    const std::wstring& getQuery() const { return m_query; }
    dbConnection* getDbConnection() const { return m_db; }

    EditMode getMode() const { return m_mode; }

    // --- Helpers ---

    // Verifică dacă avem un ID valid setat pentru editare
    bool hasValidIdentity() const {
        return !m_uniqueIdField.empty() && !m_uniqueIdValue.empty();
    }


    // --- Logică ---
    virtual void setupEditControls() = 0;
    void setupActionButtons();

    // Metodă pentru a pregăti fereastra în funcție de mod
    void updateUIForMode();

    // Această metodă va fi apelată de butonul "OK/Salvează"
    void executeAction();

   // void showModal();
  //  void centerRelativeToParent();

    // Metodă pentru a popula formularul cu date dintr-un rând selectat

    bool populateControls();
    void loadData(const std::map<std::wstring, std::wstring>& rowData);

    bool wasActionSuccessful() const { return m_actionSuccessful; }


private:
    bool deleteRecord();
};