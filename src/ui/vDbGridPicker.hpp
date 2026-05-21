#ifndef VDBGRIDPICKER_HPP
#define VDBGRIDPICKER_HPP

#include "vPanel.hpp"
#include "vEdit.hpp"
#include "vButton.hpp"
#include "../dbConnection.hpp"
#include "Layouts/Layouts.hpp"
#include "vDbFilteredGrid.hpp"

class vDbGridPicker : public vPanel {
protected:
    vEdit* m_edit = nullptr;
    vButton* m_btnOpen = nullptr;

    std::wstring m_targetQuery;
    std::wstring m_returnColumn;
    std::wstring m_returnIdColumn;
    vDbFilteredGrid* m_grid;
    dbConnection* m_db;
    std::wstring m_hiddenValue;
    std::map<std::wstring, std::wstring> m_selectedRowData;
public:
    vDbGridPicker(HINSTANCE hInst, const std::string& id, int x, int y, int width, int height,
        EventDispatcher& disp, dbConnection* db)
        : vPanel(hInst, id, x, y, width, height, disp), m_db(db)
    {
        // Nu mai avem nevoie de AnchorLayout neapărat dacă facem resize manual, 
        // dar e bine să îl lăsăm pentru consistență.
        setLayoutStrategy(std::make_unique<AnchorLayout>());
       // setLayoutStrategy(std::make_unique<FlexStackLayout>());
        
    }

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


    void create(HWND parent) override;

    void resize() override;
    // Setări lookup
    void setLookupInfo(const std::wstring& query, const std::wstring& col, const std::wstring& id_col = L"") {
        m_targetQuery = query;
        m_returnColumn = col;
        m_returnIdColumn = id_col == L"" ? col : id_col;
    }

    // Getters pentru valoare
    std::wstring getSelectedValue() const { 
        std::wstring res = m_hiddenValue == L"" ? m_edit->getText() : m_hiddenValue;
        return res; 
    }
    void setSelectedValue(const std::wstring& val) { m_hiddenValue = val; }

    std::wstring getText() const { return m_edit ? m_edit->getText() : L""; }

    void setText(const std::wstring& text) {
        if (m_edit) m_edit->setText(text);
        if (text.empty()) m_hiddenValue = L"";
    }

    void openGridSelector();
  

    void setBackgroundColor(COLORREF color) override {
        // Apelăm baza pentru a salva culoarea în m_backgroundColor (din vControl)
        vPanel::setBackgroundColor(color);

        // Dacă edit-ul și butonul sunt deja create, le aplicăm acum
        if (m_edit) m_edit->setBackgroundColor(color);
        if (m_btnOpen) m_btnOpen->setBackgroundColor(color);
    }

    void setTextColor(COLORREF color) override {
        // Apelăm baza pentru a salva culoarea în m_textColor
        vPanel::setTextColor(color);

        if (m_edit) m_edit->setTextColor(color);
        if (m_btnOpen) m_btnOpen->setTextColor(color);
    }

    void setFont(const std::wstring& face, int h, int w, bool it, bool un) override {
        vPanel::setFont(face, h, w, it, un);
        if (m_edit) m_edit->setFont(face, h, w, it, un);
        if (m_btnOpen) m_btnOpen->setFont(face, h, w, it, un);
    }

    void setDbConnection(dbConnection* db) {
        m_db = db;
    }

    void setTargetQuery(std::wstring query) {
        m_targetQuery = query;
    }

    void setReturnColumn(std::wstring column) {
        m_returnColumn = column;
    }

    void setReturnIdColumn(std::wstring column) {
        m_returnIdColumn = column;
    }

    std::wstring getReturnIdColumn( ) {
        return m_returnIdColumn ;
    }

    void setReadOnly(bool bReadOnly) {
        if (m_edit) m_edit->setReadOnly(bReadOnly);
    }

    // Metodă utilitară pentru a citi datele ulterior
    std::wstring getSelectedRowValue(const std::wstring& colName) {
        if (m_selectedRowData.count(colName)) return m_selectedRowData[colName];
        return L"";
    }
    // Metodă pentru a obține tot map-ul (dacă vrei să-l procesezi extern)
    const std::map<std::wstring, std::wstring>& getFullRowData() const {
        return m_selectedRowData;
    }

    void setValidation(const std::wstring& pattern, const std::wstring& errorMsg) {
        m_edit->setValidation(pattern, errorMsg);
    }

    bool validate() {
        return m_edit->validate();
    }

};

#endif